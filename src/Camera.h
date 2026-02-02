#pragma once
#include "Math.h"

class Camera {
public:
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::mat4 GetViewMatrix() const;
};
