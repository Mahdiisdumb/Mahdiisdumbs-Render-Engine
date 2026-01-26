#pragma once
#include <windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#include <DirectXMath.h>

using namespace Microsoft::WRL;
using namespace DirectX;

struct Vertex {
    XMFLOAT3 position;
    XMFLOAT3 color;
};

class DX12Renderer {
public:
    DX12Renderer(HWND hwnd, UINT width, UINT height);
    ~DX12Renderer();

    void Render();

private:
    void InitPipeline(HWND hwnd, UINT width, UINT height);
    void InitAssets();

    // Pipeline objects
    ComPtr<ID3D12Device> device;
    ComPtr<ID3D12CommandQueue> commandQueue;
    ComPtr<IDXGISwapChain3> swapChain;
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    ComPtr<ID3D12Resource> renderTargets[2];
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ComPtr<ID3D12GraphicsCommandList> commandList;
    ComPtr<ID3D12Fence> fence;
    HANDLE fenceEvent;
    UINT64 fenceValue;
    UINT rtvDescriptorSize;
    UINT frameIndex;

    // Assets
    ComPtr<ID3D12Resource> vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

    // Synchronization
    void WaitForPreviousFrame();
};