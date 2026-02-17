#include "Model.h"
#include <iostream>
#if !defined(USE_ASSIMP)
// Prefer enabling via project settings; do not force-enable globally if project defines it.
// To enable Assimp linking, define USE_ASSIMP in your project settings and link the assimp library.
#endif
#ifdef USE_ASSIMP
#endif

bool Model::LoadFBX(const std::string& path, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList) {
    std::cout << "Loading FBX: " << path << std::endl;
#ifdef USE_ASSIMP
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs);
    if (!scene || !scene->HasMeshes()) {
        std::cerr << "Assimp failed to load or no meshes: " << importer.GetErrorString() << std::endl;
        return false;
    }

    // For now, load only the first mesh
    aiMesh* mesh = scene->mMeshes[0];
    vertices.clear();
    indices.clear();

    vertices.reserve(mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        MeshVertex v;
        v.position.x = mesh->mVertices[i].x;
        v.position.y = mesh->mVertices[i].y;
        v.position.z = mesh->mVertices[i].z;
        if (mesh->HasNormals()) {
            v.normal.x = mesh->mNormals[i].x;
            v.normal.y = mesh->mNormals[i].y;
            v.normal.z = mesh->mNormals[i].z;
        } else {
            v.normal = {0.0f, 0.0f, 0.0f};
        }
        if (mesh->HasTextureCoords(0)) {
            v.uv.x = mesh->mTextureCoords[0][i].x;
            v.uv.y = mesh->mTextureCoords[0][i].y;
        } else {
            v.uv = {0.0f, 0.0f};
        }
        vertices.push_back(v);
    }

    // collect indices
    for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
        aiFace& face = mesh->mFaces[f];
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // bones
    bones.clear();
    if (mesh->mNumBones > 0) {
        bones.resize(mesh->mNumBones);
        for (unsigned int b = 0; b < mesh->mNumBones; ++b) {
            bones[b].position = math::vec3{0.0f, 0.0f, 0.0f};
            bones[b].rotation = math::quat{0.0f, 0.0f, 0.0f, 1.0f};
        }
    }

    // Try to load diffuse texture from material
    if (scene->HasMaterials()) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            aiString texPath;
                if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
                    std::string tpath = texPath.C_Str();
                // If path is relative, make it relative to FBX file
                if (tpath.size() > 0 && tpath[0] != '/' && !(tpath.size() > 1 && tpath[1] == ':')) {
                    size_t pos = path.find_last_of("/\\");
                    std::string base = (pos == std::string::npos) ? std::string() : path.substr(0, pos+1);
                    tpath = base + tpath;
                }
                // Caller (renderer) will create SRV; here we only load the texture resource via Texture class if device/cmdList available
                // but this overload does not have device; the renderer should call LoadTexture separately.
                    std::cout << "Found diffuse texture: " << tpath << std::endl;
                    pendingTexturePath = tpath;
            }
        }
    }

    return true;
#else
    std::cerr << "Assimp not enabled; cannot load FBX. Define USE_ASSIMP.\n";
    return false;
#endif
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
