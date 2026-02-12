#pragma once
#include "Math.h"
#include "Texture.h"
#include "Bone.h"
#include <vector>
#include <string>
#include "d3dx12.h"


class Model {
public:
    bool LoadFBX(const std::string& path);
    bool LoadTexture(const std::string& path, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
    void Draw();
    void ApplyBoneTransform(int boneIndex, const glm::vec3& pos, const glm::quat& rot);

public:
    std::vector<Bone> bones;
    Texture texture;
};
