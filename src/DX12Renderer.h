#pragma once
#include <dxgi1_6.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <directx/d3dx12.h>

using namespace Microsoft::WRL;
using namespace DirectX;

struct Vertex {
    XMFLOAT3 position;
    XMFLOAT4 color;
};

struct TransformData {
    XMFLOAT4X4 worldViewProj;
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
    ComPtr<ID3D12Resource> constantBuffer;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW indexBufferView;
    TransformData* pCbData = nullptr;
    UINT indexCount = 0;
    float rotationY = 0.0f;

    UINT rtvDescriptorSize = 0;
    UINT frameIndex = 0;
    UINT64 fenceValue = 0;
};
