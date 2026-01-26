#include "DX12Renderer.h"
#include <stdexcept>
#include <iostream>

DX12Renderer::DX12Renderer(HWND hwnd, UINT width, UINT height) {
    InitPipeline(hwnd, width, height);
    InitAssets();
}

DX12Renderer::~DX12Renderer() {
    WaitForPreviousFrame();
    CloseHandle(fenceEvent);
}

void DX12Renderer::InitPipeline(HWND hwnd, UINT width, UINT height) {
    UINT dxgiFactoryFlags = 0;

    ComPtr<IDXGIFactory6> factory;
    if (FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory))))
        throw std::runtime_error("Failed to create DXGI Factory");

    if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device))))
        throw std::runtime_error("Failed to create D3D12 Device");

    // Command queue
    D3D12_COMMAND_QUEUE_DESC cqDesc = {};
    cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    if (FAILED(device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&commandQueue))))
        throw std::runtime_error("Failed to create command queue");

    // Swap chain
    DXGI_SWAP_CHAIN_DESC1 scDesc = {};
    scDesc.BufferCount = 2;
    scDesc.Width = width;
    scDesc.Height = height;
    scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain1;
    if (FAILED(factory->CreateSwapChainForHwnd(
        commandQueue.Get(), hwnd, &scDesc, nullptr, nullptr, &swapChain1)))
        throw std::runtime_error("Failed to create swap chain");

    swapChain1.As(&swapChain);
    frameIndex = swapChain->GetCurrentBackBufferIndex();

    // RTV Heap
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 2;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (FAILED(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap))))
        throw std::runtime_error("Failed to create RTV heap");

    rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // Create frame resources
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < 2; i++) {
        swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
        device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, rtvDescriptorSize);
    }

    // Command allocator
    if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator))))
        throw std::runtime_error("Failed to create command allocator");
}

void DX12Renderer::InitAssets() {
    if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr,
                                         IID_PPV_ARGS(&commandList))))
        throw std::runtime_error("Failed to create command list");

    // Cube vertices
    Vertex vertices[] = {
        {{-0.5f,-0.5f,-0.5f},{1,0,0}}, {{-0.5f,+0.5f,-0.5f},{0,1,0}}, {{+0.5f,+0.5f,-0.5f},{0,0,1}}, {{+0.5f,-0.5f,-0.5f},{1,1,0}},
        {{-0.5f,-0.5f,+0.5f},{1,0,1}}, {{-0.5f,+0.5f,+0.5f},{0,1,1}}, {{+0.5f,+0.5f,+0.5f},{1,1,1}}, {{+0.5f,-0.5f,+0.5f},{0,0,0}}
    };

    const UINT vertexBufferSize = sizeof(vertices);

    // Create default heap
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Width = vertexBufferSize;
    resDesc.Height = 1;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.SampleDesc.Count = 1;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    if (FAILED(device->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexBuffer))))
        throw std::runtime_error("Failed to create vertex buffer");

    // Copy data
    UINT8* pVertexDataBegin;
    D3D12_RANGE readRange = {};
    vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
    memcpy(pVertexDataBegin, vertices, sizeof(vertices));
    vertexBuffer->Unmap(0, nullptr);

    vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes = sizeof(Vertex);
    vertexBufferView.SizeInBytes = vertexBufferSize;

    commandList->Close();

    // Fence
    device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    fenceValue = 1;
    fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (fenceEvent == nullptr) throw std::runtime_error("Failed to create fence event");
}

void DX12Renderer::Render() {
    // For now just clear to dark gray
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);
    FLOAT clearColor[] = {0.2f, 0.2f, 0.2f, 1.0f};
    commandAllocator->Reset();
    commandList->Reset(commandAllocator.Get(), nullptr);
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(),
                                                                         D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(),
                                                                         D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
    commandList->Close();
    ID3D12CommandList* ppCommandLists[] = {commandList.Get()};
    commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    swapChain->Present(1, 0);
    WaitForPreviousFrame();
}

void DX12Renderer::WaitForPreviousFrame() {
    const UINT64 fenceToWaitFor = fenceValue;
    commandQueue->Signal(fence.Get(), fenceToWaitFor);
    fenceValue++;

    if (fence->GetCompletedValue() < fenceToWaitFor) {
        fence->SetEventOnCompletion(fenceToWaitFor, fenceEvent);
        WaitForSingleObject(fenceEvent, INFINITE);
    }

    frameIndex = swapChain->GetCurrentBackBufferIndex();
}