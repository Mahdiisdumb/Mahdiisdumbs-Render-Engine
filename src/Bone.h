#pragma once
#include "Math.h"

struct Bone {
    math::vec3 position{0.0f, 0.0f, 0.0f};
    math::quat rotation{0.0f, 0.0f, 0.0f, 1.0f};
    void UpdateGlobal(); // implement in Bone.cpp
};
