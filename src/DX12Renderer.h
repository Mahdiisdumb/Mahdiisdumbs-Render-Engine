#pragma once
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "d3dx12.h"
#include <wrl.h>
using namespace Microsoft::WRL;

class DX12Renderer {
public:
    bool Init(HWND hwnd);
    void RenderCube();
    void Shutdown();

private:
    ComPtr<ID3D12Device> device;
    ComPtr<IDXGISwapChain3> swapChain;
    ComPtr<ID3D12CommandQueue> cmdQueue;
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    ComPtr<ID3D12CommandAllocator> cmdAllocator;
    ComPtr<ID3D12GraphicsCommandList> cmdList;

    UINT rtvDescriptorSize;
    int frameIndex;
};
