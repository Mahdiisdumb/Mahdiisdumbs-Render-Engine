#pragma once
// Lightweight math header: prefer DirectXMath. If GLM is available, project can include it separately.
#include <DirectXMath.h>

namespace math {
    using namespace DirectX;
    using vec3 = XMFLOAT3;
    using quat = XMFLOAT4; // (x,y,z,w)
}
