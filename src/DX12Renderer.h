#pragma once
#include <dxgi1_6.h>
#include <wrl.h>
#include <DirectXMath.h>
#include "directx/d3dx12.h"
#include "Camera.h"
#include "Model.h"

using namespace Microsoft::WRL;
using namespace DirectX;

struct Vertex {
    XMFLOAT3 position;
    XMFLOAT4 color;
    XMFLOAT2 uv;
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
    void UpdateCamera(float dt);

    // expose for other systems
    ID3D12Device* GetDevice() { return device.Get(); }
    ID3D12GraphicsCommandList* GetCommandList() { return cmdList.Get(); }
    ID3D12CommandQueue* GetCommandQueue() { return cmdQueue.Get(); }
    bool LoadModel(const std::string& path);
    bool LoadTexture(const std::string& path);

    Camera camera;
    Model model;

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
    ComPtr<ID3D12DescriptorHeap> srvHeap;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW indexBufferView;
    TransformData* pCbData = nullptr;
    UINT indexCount = 0;
    float rotationY = 0.0f;

    UINT rtvDescriptorSize = 0;
    UINT frameIndex = 0;
    UINT64 fenceValue = 0;
};
