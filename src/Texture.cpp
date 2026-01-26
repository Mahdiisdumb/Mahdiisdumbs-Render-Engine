#define STB_IMAGE_IMPLEMENTATION
#include "Texture.h"
#include <stb_image.h>
#include <iostream>

bool Texture::LoadFromFile(const std::string& path, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList) {
    int width, height, channels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 4);
    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return false;
    }
    stbi_image_free(data);
    return true;
}
