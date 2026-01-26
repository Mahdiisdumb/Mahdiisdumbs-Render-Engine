#pragma once
#include <d3d12.h>
#include <string>

class Texture {
public:
    bool LoadFromFile(const std::string& path, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
private:
    // Implementation-specific members
};
