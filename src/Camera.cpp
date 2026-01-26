#include "Camera.h"
#include <glm/gtx/euler_angles.hpp>

glm::mat4 Camera::GetViewMatrix() const {
    glm::mat4 rot = glm::yawPitchRoll(rotation.y, rotation.x, rotation.z);
    glm::mat4 trans = glm::translate(glm::mat4(1.0f), -position);
    return rot * trans;
}
