#pragma once
#include <string>
#include "directx/d3dx12.h"


class Texture {
public:
    bool LoadFromFile(const std::string& path, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
private:
    // Implementation-specific members
};
