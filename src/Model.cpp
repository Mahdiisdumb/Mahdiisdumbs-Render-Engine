#include "Model.h"
#include <iostream>
#ifdef USE_ASSIMP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#endif

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

void Model::ApplyBoneTransform(int boneIndex, const math::vec3& pos, const math::quat& rot) {
    if (boneIndex < 0 || boneIndex >= bones.size()) return;
    // convert math::vec3 to glm style is omitted; Bone uses math types too in this simplified setup
    bones[boneIndex].position.x = pos.x;
    bones[boneIndex].position.y = pos.y;
    bones[boneIndex].position.z = pos.z;
    bones[boneIndex].rotation.x = rot.x;
    bones[boneIndex].rotation.y = rot.y;
    bones[boneIndex].rotation.z = rot.z;
    bones[boneIndex].rotation.w = rot.w;
}
