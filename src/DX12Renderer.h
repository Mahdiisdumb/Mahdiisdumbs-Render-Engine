#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <DirectXMath.h>
#include "d3dx12/d3dx12.h"  // Make sure path matches

using namespace Microsoft::WRL;
using namespace DirectX;

struct Vertex {
    XMFLOAT3 position;
    XMFLOAT4 color;
};

class DX12Renderer {
public:
    DX12Renderer() = default;
    ~DX12Renderer() = default;

    bool Init(HWND hwnd);
    void Render();

private:
    void CreateCubeMesh();
    
    HWND mHwnd = nullptr;

    // DX12 objects
    ComPtr<ID3D12Device> device;
    ComPtr<ID3D12CommandQueue> cmdQueue;
    ComPtr<IDXGISwapChain3> swapChain;
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    ComPtr<ID3D12CommandAllocator> cmdAllocator;
    ComPtr<ID3D12GraphicsCommandList> cmdList;
    ComPtr<ID3D12Fence> fence;
    ComPtr<ID3D12PipelineState> pipelineState;
    ComPtr<ID3D12RootSignature> rootSignature;

    // Mesh data
    ComPtr<ID3D12Resource> vertexBuffer;
    ComPtr<ID3D12Resource> indexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW indexBufferView;
    UINT indexCount = 0;

    UINT rtvDescriptorSize = 0;
    UINT frameIndex = 0;
    UINT64 fenceValue = 0;
};
