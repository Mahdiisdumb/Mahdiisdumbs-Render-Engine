#include <windows.h>
#include "DX12Renderer.h"
#include <iostream>

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int main() {
    try {
        HINSTANCE hInstance = GetModuleHandle(nullptr);

        // Register window class
        WNDCLASSW wc = {};
        wc.lpfnWndProc = WndProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = L"MahdiisdumbsRE";
        RegisterClassW(&wc);

        // Create window
        HWND hwnd = CreateWindowExW(
            0,
            wc.lpszClassName,
            L"Mahdiisdumbs Render Engine",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
            nullptr, nullptr, hInstance, nullptr
        );

        if (!hwnd) {
            std::cerr << "Failed to create window\n";
            return -1;
        }

        ShowWindow(hwnd, SW_SHOW);

        // Initialize DX12 renderer
        DX12Renderer renderer;
        if (!renderer.Init(hwnd)) {
            std::cerr << "Failed to initialize renderer\n";
            return -1;
        }

        // Main message loop
        MSG msg = {};
        while (msg.message != WM_QUIT) {
            if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else {
                renderer.Render();
            }
        }

        return static_cast<int>(msg.wParam);
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << "\n";
        return -1;
    }
}