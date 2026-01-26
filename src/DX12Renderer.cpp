#include "DX12Renderer.h"
#include <stdexcept>

bool DX12Renderer::Init(HWND hwnd) {
    // Device
    if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)))) {
        throw std::runtime_error("Failed to create device");
    }

    // Command queue
    D3D12_COMMAND_QUEUE_DESC qDesc = {};
    qDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    if (FAILED(device->CreateCommandQueue(&qDesc, IID_PPV_ARGS(&cmdQueue)))) {
        throw std::runtime_error("Failed to create command queue");
    }

    // RTV Heap
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = 2;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeap));
    rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // Command allocator & list
    device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAllocator));
    device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAllocator.Get(), nullptr, IID_PPV_ARGS(&cmdList));

    frameIndex = 0;
    return true;
}

void DX12Renderer::RenderCube() {
    // Simple placeholder: clear screen
    FLOAT clearColor[] = {0.2f, 0.3f, 0.3f, 1.0f};
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        nullptr, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        nullptr, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    cmdList->Close();
    ID3D12CommandList* ppCommandLists[] = { cmdList.Get() };
    cmdQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    cmdAllocator->Reset();
    cmdList->Reset(cmdAllocator.Get(), nullptr);
}

void DX12Renderer::Shutdown() {
    cmdList->Release();
    cmdAllocator->Release();
    rtvHeap->Release();
    cmdQueue->Release();
    swapChain->Release();
    device->Release();
}
