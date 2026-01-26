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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    try {
        WNDCLASSW wc = {};
        wc.lpfnWndProc = WndProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = L"MahdiisdumbsRE";

        RegisterClassW(&wc);

        HWND hwnd = CreateWindowExW(
            0,
            wc.lpszClassName,
            L"Mahdiisdumbs Render Engine",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
            nullptr, nullptr, hInstance, nullptr
        );

        ShowWindow(hwnd, nCmdShow);

        DX12Renderer renderer;
        if (!renderer.Init(hwnd)) {
            MessageBoxW(nullptr, L"Failed to initialize renderer", L"Error", MB_OK);
            return -1;
        }

        MSG msg = {};
        while (msg.message != WM_QUIT) {
            if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            } else {
                renderer.Render();
            }
        }

        return 0;
    }
    catch (const std::exception& e) {
        MessageBoxA(nullptr, e.what(), "Fatal Error", MB_OK | MB_ICONERROR);
        return -1;
    }
}
