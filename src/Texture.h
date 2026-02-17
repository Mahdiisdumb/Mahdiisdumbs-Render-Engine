#pragma once
#include <string>
#include <wrl.h>
#include "directx/d3dx12.h"
#define USE_ASSIMP

using Microsoft::WRL::ComPtr;

class Texture {
public:
    bool LoadFromFile(const std::string& path, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
    ID3D12Resource* GetResource() { return texture.Get(); }
private:
    ComPtr<ID3D12Resource> texture;
};
