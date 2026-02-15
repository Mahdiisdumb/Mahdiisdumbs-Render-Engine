#include "DX12Renderer.h"
#include <iostream>

bool DX12Renderer::Init(HWND hwnd) {
    // Minimal init to satisfy linker; the real implementation is in DX12Renderer.cpp
    // If the real implementation is part of the build this stub will be ignored.
    std::cout << "DX12Renderer::Init (stub)" << std::endl;
    return true;
}

void DX12Renderer::Render() {
    // stub: nothing
}

void DX12Renderer::UpdateCamera(float dt) {
    // stub: nothing
}
