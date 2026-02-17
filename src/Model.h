#pragma once
#include "Math.h"
#include "Texture.h"
#include "Bone.h"
#include <vector>
#include <string>
#include "directx/d3dx12.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model {
public:
    bool LoadFBX(const std::string& path, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
    bool LoadTexture(const std::string& path, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
    void Draw();
    void ApplyBoneTransform(int boneIndex, const math::vec3& pos, const math::quat& rot);

public:
    std::vector<Bone> bones;
    Texture texture;
    struct MeshVertex {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT2 uv;
    };
    std::vector<MeshVertex> vertices;
    std::vector<uint32_t> indices;
    std::string pendingTexturePath; // path found by Assimp; renderer can load it
};
