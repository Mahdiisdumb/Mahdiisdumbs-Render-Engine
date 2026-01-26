#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include "Scene.h"
#include "Camera.h"

class Renderer {
public:
    void Init(HWND hwnd);
    void RenderScene(Scene& scene, Camera& cam);
    void Shutdown();
    ID3D12Device* GetDevice() { return device; }
    ID3D12CommandQueue* GetCommandQueue() { return cmdQueue; }
private:
    ID3D12Device* device;
    ID3D12CommandQueue* cmdQueue;
    IDXGISwapChain3* swapChain;
    // More DX12 buffers/pipeline here...
};