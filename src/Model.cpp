#include "Model.h"
#include <iostream>

bool Model::LoadFBX(const std::string& path) {
    std::cout << "Loading FBX: " << path << std::endl;
    return true; // placeholder
}

bool Model::LoadTexture(const std::string& path, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList) {
    return texture.LoadFromFile(path, device, cmdList);
}

void Model::Draw() {
    // placeholder: send vertex + bone matrices to GPU
}

void Model::ApplyBoneTransform(int boneIndex, const glm::vec3& pos, const glm::quat& rot) {
    if (boneIndex < 0 || boneIndex >= bones.size()) return;
    bones[boneIndex].position = pos;
    bones[boneIndex].rotation = rot;
}
