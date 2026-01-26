#pragma once
#include <vector>
#include "Model.h"

class Scene {
public:
    std::vector<Model*> models;
    void AddModel(Model* m) { models.push_back(m); }
    void UpdateBones() {
        for (auto m : models)
            for (auto& b : m->bones)
                b.UpdateGlobal();
    }
};
