#pragma once
#include <DirectXMath.h>

class Camera {
public:
    Camera() = default;
    DirectX::XMFLOAT3 position{0.0f, 0.0f, -2.0f};
    DirectX::XMFLOAT3 rotation{0.0f, 0.0f, 0.0f}; // pitch, yaw, roll in radians

    DirectX::XMMATRIX GetViewMatrix() const;
};
