#include "Math.h"
#include "Model.h"
#include "Camera.h"
#include "ImguiWrapper.h"
#include <windows.h>
#include <d3d12.h>
#include <iostream>

int main() {
    std::cout << "Mahdiisdumbs Render Engine starting..." << std::endl;

    Camera cam;
    Model model;

    model.LoadFBX("test.fbx");

    ImGuiWrapper::Init();

    std::cout << "Engine initialized." << std::endl;

    ImGuiWrapper::Shutdown();
    return 0;
}
