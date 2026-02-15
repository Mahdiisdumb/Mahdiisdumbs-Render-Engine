#include "Camera.h"
#include <DirectXMath.h>

DirectX::XMMATRIX Camera::GetViewMatrix() const {
    using namespace DirectX;
    XMVECTOR pos = XMLoadFloat3(&position);
    XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
    XMVECTOR forward = XMVector3TransformNormal(XMVectorSet(0, 0, 1, 0), rotMat);
    XMVECTOR target = XMVectorAdd(pos, forward);
    XMVECTOR up = XMVector3TransformNormal(XMVectorSet(0,1,0,0), rotMat);
    return XMMatrixLookAtLH(pos, target, up);
}
