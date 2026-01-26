#include "DX12Renderer.h"
#include <stdexcept>
#include <vector>
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

// Simple vertex shader
const char* vertexShader = R"(
struct VS_INPUT {
    float3 position : POSITION;
    float4 color : COLOR;
};

struct PS_INPUT {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PS_INPUT main(VS_INPUT input) {
    PS_INPUT output;
    output.position = float4(input.position * 2.0f, 1.0f);
    output.color = input.color;
    return output;
}
)";

// Simple pixel shader
const char* pixelShader = R"(
struct PS_INPUT {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

float4 main(PS_INPUT input) : SV_TARGET {
    return input.color;
}
)";

void DX12Renderer::CreateCubeMesh() {
    // Create cube vertices (pink color)
    Vertex cubeVertices[] = {
        // Front face
        {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.75f, 0.8f, 1.0f}},  // Pink
        {{-0.5f,  0.5f, -0.5f}, {1.0f, 0.75f, 0.8f, 1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.75f, 0.8f, 1.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.75f, 0.8f, 1.0f}},
        // Back face
        {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.75f, 0.8f, 1.0f}},
        {{-0.5f,  0.5f,  0.5f}, {1.0f, 0.75f, 0.8f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.75f, 0.8f, 1.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.75f, 0.8f, 1.0f}},
    };

    // Create cube indices
    UINT16 cubeIndices[] = {
        // Front
        0, 1, 2,  0, 2, 3,
        // Right
        3, 2, 6,  3, 6, 7,
        // Back
        7, 6, 5,  7, 5, 4,
        // Left
        4, 5, 1,  4, 1, 0,
        // Top
        1, 5, 6,  1, 6, 2,
        // Bottom
        4, 0, 3,  4, 3, 7,
    };

    indexCount = _countof(cubeIndices);

    // Create vertex buffer
    D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(cubeVertices));

    if (FAILED(device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&vertexBuffer)))) {
        throw std::runtime_error("Failed to create vertex buffer");
    }

    // Create index buffer
    resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(cubeIndices));
    if (FAILED(device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&indexBuffer)))) {
        throw std::runtime_error("Failed to create index buffer");
    }

    // Upload data using a temporary upload buffer
    ComPtr<ID3D12Resource> vertexUploadBuffer;
    D3D12_HEAP_PROPERTIES uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    D3D12_RESOURCE_DESC uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(cubeVertices));

    if (FAILED(device->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &uploadDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&vertexUploadBuffer)))) {
        throw std::runtime_error("Failed to create vertex upload buffer");
    }

    ComPtr<ID3D12Resource> indexUploadBuffer;
    uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(cubeIndices));
    if (FAILED(device->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &uploadDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&indexUploadBuffer)))) {
        throw std::runtime_error("Failed to create index upload buffer");
    }

    // Copy vertex data
    void* mappedData;
    vertexUploadBuffer->Map(0, nullptr, &mappedData);
    memcpy(mappedData, cubeVertices, sizeof(cubeVertices));
    vertexUploadBuffer->Unmap(0, nullptr);

    // Copy index data
    indexUploadBuffer->Map(0, nullptr, &mappedData);
    memcpy(mappedData, cubeIndices, sizeof(cubeIndices));
    indexUploadBuffer->Unmap(0, nullptr);

    // Record copy commands
    ComPtr<ID3D12CommandAllocator> uploadAllocator;
    if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&uploadAllocator)))) {
        throw std::runtime_error("Failed to create upload allocator");
    }

    ComPtr<ID3D12GraphicsCommandList> uploadCmdList;
    if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, uploadAllocator.Get(), nullptr, IID_PPV_ARGS(&uploadCmdList)))) {
        throw std::runtime_error("Failed to create upload command list");
    }

    uploadCmdList->CopyBufferRegion(vertexBuffer.Get(), 0, vertexUploadBuffer.Get(), 0, sizeof(cubeVertices));
    uploadCmdList->CopyBufferRegion(indexBuffer.Get(), 0, indexUploadBuffer.Get(), 0, sizeof(cubeIndices));

    // Transition to vertex/index buffer state
    CD3DX12_RESOURCE_BARRIER vertexBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    uploadCmdList->ResourceBarrier(1, &vertexBarrier);

    CD3DX12_RESOURCE_BARRIER indexBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
    uploadCmdList->ResourceBarrier(1, &indexBarrier);

    if (FAILED(uploadCmdList->Close())) {
        throw std::runtime_error("Failed to close upload command list");
    }

    // Execute upload
    ID3D12CommandList* lists[] = {uploadCmdList.Get()};
    cmdQueue->ExecuteCommandLists(_countof(lists), lists);

    // Wait for upload to complete
    ComPtr<ID3D12Fence> uploadFence;
    if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&uploadFence)))) {
        throw std::runtime_error("Failed to create upload fence");
    }

    cmdQueue->Signal(uploadFence.Get(), 1);
    while (uploadFence->GetCompletedValue() < 1) {
        Sleep(1);
    }

    // Set up views
    vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes = sizeof(Vertex);
    vertexBufferView.SizeInBytes = sizeof(cubeVertices);

    indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
    indexBufferView.Format = DXGI_FORMAT_R16_UINT;
    indexBufferView.SizeInBytes = sizeof(cubeIndices);
}

bool DX12Renderer::Init(HWND hwnd) {
    mHwnd = hwnd;

    // 1. Create device
    if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device)))) {
        throw std::runtime_error("Failed to create D3D12 device");
    }

    // 2. Create command queue
    D3D12_COMMAND_QUEUE_DESC cqDesc = {};
    cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    if (FAILED(device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&cmdQueue)))) {
        throw std::runtime_error("Failed to create command queue");
    }

    // 3. Swap chain
    DXGI_SWAP_CHAIN_DESC1 scDesc = {};
    scDesc.BufferCount = 2;
    scDesc.Width = 800;
    scDesc.Height = 600;
    scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scDesc.SampleDesc.Count = 1;

    ComPtr<IDXGIFactory6> factory;
    CreateDXGIFactory1(IID_PPV_ARGS(&factory));

    ComPtr<IDXGISwapChain1> tempSwap;
    if (FAILED(factory->CreateSwapChainForHwnd(
        cmdQueue.Get(), mHwnd, &scDesc, nullptr, nullptr, &tempSwap))) {
        throw std::runtime_error("Failed to create swap chain");
    }

    tempSwap.As(&swapChain);
    frameIndex = swapChain->GetCurrentBackBufferIndex();

    // 4. RTV descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 2;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (FAILED(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)))) {
        throw std::runtime_error("Failed to create RTV heap");
    }

    rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // 4b. Create RTVs for swap chain buffers
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < 2; ++i) {
        ComPtr<ID3D12Resource> buffer;
        if (FAILED(swapChain->GetBuffer(i, IID_PPV_ARGS(&buffer)))) {
            throw std::runtime_error("Failed to get swap chain buffer");
        }
        device->CreateRenderTargetView(buffer.Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, rtvDescriptorSize);
    }

    // 5. Fence for synchronization
    if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)))) {
        throw std::runtime_error("Failed to create fence");
    }
    fenceValue = 1;

    // 6. Command allocator & command list
    if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAllocator)))) {
        throw std::runtime_error("Failed to create command allocator");
    }

    if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAllocator.Get(), nullptr, IID_PPV_ARGS(&cmdList)))) {
        throw std::runtime_error("Failed to create command list");
    }

    cmdList->Close(); // Close initially

    // 7. Create root signature
    D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
    rootSigDesc.NumParameters = 0;
    rootSigDesc.pParameters = nullptr;
    rootSigDesc.NumStaticSamplers = 0;
    rootSigDesc.pStaticSamplers = nullptr;
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    if (FAILED(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error))) {
        throw std::runtime_error("Failed to serialize root signature");
    }

    if (FAILED(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)))) {
        throw std::runtime_error("Failed to create root signature");
    }

    // 8. Compile shaders
    ComPtr<ID3DBlob> vsBlob, psBlob;
    ComPtr<ID3DBlob> vsError, psError;
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

    if (FAILED(D3DCompile(vertexShader, strlen(vertexShader), nullptr, nullptr, nullptr, 
        "main", "vs_5_0", compileFlags, 0, &vsBlob, &vsError))) {
        std::string errMsg = "Vertex shader compile error: ";
        if (vsError) {
            errMsg += (const char*)vsError->GetBufferPointer();
        }
        throw std::runtime_error(errMsg);
    }

    if (FAILED(D3DCompile(pixelShader, strlen(pixelShader), nullptr, nullptr, nullptr,
        "main", "ps_5_0", compileFlags, 0, &psBlob, &psError))) {
        std::string errMsg = "Pixel shader compile error: ";
        if (psError) {
            errMsg += (const char*)psError->GetBufferPointer();
        }
        throw std::runtime_error(errMsg);
    }

    // 9. Create pipeline state
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = rootSignature.Get();
    psoDesc.VS = {vsBlob->GetBufferPointer(), vsBlob->GetBufferSize()};
    psoDesc.PS = {psBlob->GetBufferPointer(), psBlob->GetBufferSize()};
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;

    // Input layout
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };
    psoDesc.InputLayout = {inputLayout, _countof(inputLayout)};
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    if (FAILED(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)))) {
        throw std::runtime_error("Failed to create pipeline state");
    }

    // 10. Create cube mesh
    CreateCubeMesh();

    return true;
}

void DX12Renderer::Render() {
    // Clear color - Black background
    static const FLOAT clearColor[] = {0.0f, 0.0f, 0.0f, 1.0f};

    cmdAllocator->Reset();
    cmdList->Reset(cmdAllocator.Get(), nullptr);

    // Get the current back buffer
    frameIndex = swapChain->GetCurrentBackBufferIndex();
    ComPtr<ID3D12Resource> backBuffer;
    swapChain->GetBuffer(frameIndex, IID_PPV_ARGS(&backBuffer));

    // Get RTV handle for current frame
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);

    CD3DX12_RESOURCE_BARRIER barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
        backBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    cmdList->ResourceBarrier(1, &barrier1);

    cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // Set pipeline state and render target
    cmdList->SetPipelineState(pipelineState.Get());
    cmdList->SetGraphicsRootSignature(rootSignature.Get());
    
    D3D12_VIEWPORT viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, 800.0f, 600.0f);
    cmdList->RSSetViewports(1, &viewport);
    
    D3D12_RECT scissorRect = CD3DX12_RECT(0, 0, 800, 600);
    cmdList->RSSetScissorRects(1, &scissorRect);
    
    cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Render cube
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->IASetVertexBuffers(0, 1, &vertexBufferView);
    cmdList->IASetIndexBuffer(&indexBufferView);
    cmdList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);

    CD3DX12_RESOURCE_BARRIER barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
        backBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    cmdList->ResourceBarrier(1, &barrier2);

    cmdList->Close();
    ID3D12CommandList* lists[] = {cmdList.Get()};
    cmdQueue->ExecuteCommandLists(_countof(lists), lists);

    // Present and sync
    if (FAILED(swapChain->Present(1, 0))) {
        throw std::runtime_error("Failed to present");
    }

    // Wait for GPU to finish
    UINT64 lastFenceValue = fenceValue;
    if (FAILED(cmdQueue->Signal(fence.Get(), lastFenceValue))) {
        throw std::runtime_error("Failed to signal fence");
    }
    fenceValue++;

    if (fence->GetCompletedValue() < lastFenceValue) {
        HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (fenceEvent == nullptr) {
            throw std::runtime_error("Failed to create fence event");
        }
        if (FAILED(fence->SetEventOnCompletion(lastFenceValue, fenceEvent))) {
            CloseHandle(fenceEvent);
            throw std::runtime_error("Failed to set fence event");
        }
        WaitForSingleObject(fenceEvent, INFINITE);
        CloseHandle(fenceEvent);
    }
}
