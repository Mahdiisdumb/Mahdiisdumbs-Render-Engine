#include <windows.h>
#include "DX12Renderer.h"
#include "ImguiWrapper.h"
#include <iostream>
#include <DbgHelp.h>

#pragma comment(lib, "dbghelp.lib")

static LONG WINAPI MyExceptionFilter(EXCEPTION_POINTERS* pExceptionPointers) {
    // Write a minidump to disk for post-mortem analysis
    HANDLE hFile = CreateFileA("crash.dmp", GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION mei;
        mei.ThreadId = GetCurrentThreadId();
        mei.ExceptionPointers = pExceptionPointers;
        mei.ClientPointers = FALSE;
        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpWithDataSegs, &mei, nullptr, nullptr);
        CloseHandle(hFile);
    }
    MessageBoxA(nullptr, "The application crashed. A crash dump was written to crash.dmp", "Crash", MB_OK | MB_ICONERROR);
    return EXCEPTION_EXECUTE_HANDLER;
}

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
        SetUnhandledExceptionFilter(MyExceptionFilter);
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

        // Initialize ImGui (optional)
        ImGuiWrapper::Init(hwnd, renderer.GetDevice(), renderer.GetCommandQueue(), DXGI_FORMAT_R8G8B8A8_UNORM); // Re-initialized ImGui

        // Main message loop
        MSG msg = {};
        while (msg.message != WM_QUIT) {
            if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else {
                ImGuiWrapper::NewFrame();

                // Hotkeys handling
                bool ctrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
                bool alt = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;

                // Ctrl+O -> Open model
                static bool prevCtrlO = false;
                bool curCtrlO = (GetAsyncKeyState('O') & 0x8000) != 0 && ctrl;
                if (curCtrlO && !prevCtrlO) {
                    OPENFILENAMEA ofn;
                    CHAR szFile[MAX_PATH] = {0};
                    ZeroMemory(&ofn, sizeof(ofn));
                    ofn.lStructSize = sizeof(ofn);
                    ofn.hwndOwner = hwnd;
                    ofn.lpstrFile = szFile;
                    ofn.nMaxFile = MAX_PATH;
                    ofn.lpstrFilter = "Model Files\0*.fbx;*.obj\0All\0*.*\0";
                    ofn.Flags = OFN_FILEMUSTEXIST;
                    if (GetOpenFileNameA(&ofn)) {
                        renderer.LoadModel(szFile);
                        std::cout << "Loaded model: " << szFile << std::endl;
                    }
                }
                prevCtrlO = curCtrlO;

                // Ctrl+T -> Open texture
                static bool prevCtrlT = false;
                bool curCtrlT = (GetAsyncKeyState('T') & 0x8000) != 0 && ctrl;
                if (curCtrlT && !prevCtrlT) {
                    OPENFILENAMEA ofn;
                    CHAR szFile[MAX_PATH] = {0};
                    ZeroMemory(&ofn, sizeof(ofn));
                    ofn.lStructSize = sizeof(ofn);
                    ofn.hwndOwner = hwnd;
                    ofn.lpstrFile = szFile;
                    ofn.nMaxFile = MAX_PATH;
                    ofn.lpstrFilter = "Image Files\0*.png;*.jpg;*.jpeg;*.dds\0All\0*.*\0";
                    ofn.Flags = OFN_FILEMUSTEXIST;
                    if (GetOpenFileNameA(&ofn)) {
                        renderer.LoadTexture(szFile);
                        std::cout << "Loaded texture: " << szFile << std::endl;
                    }
                }
                prevCtrlT = curCtrlT;

                // Ctrl+Alt+C -> Toggle color picker
                static bool prevColor = false;
                bool curColor = (GetAsyncKeyState('C') & 0x8000) != 0 && ctrl && alt;
                if (curColor && !prevColor) {
                    ImGuiWrapper::g_showColorPicker = !ImGuiWrapper::g_showColorPicker;
                }
                prevColor = curColor;

                ImGuiWrapper::BeginWindow("Inspector");
                ImGuiWrapper::Text("Use WASD + RMB to move and look. Shift to sprint.");
                ImGuiWrapper::EndWindow();

                renderer.UpdateCamera(1.0f / 60.0f);
                renderer.Render();

                // Render ImGui UI and helper windows
                ImGuiWrapper::Render(renderer.GetCommandList());
                ImGuiWrapper::RenderImportUI(&renderer);
                ImGuiWrapper::RenderColorPicker();
            }
        }

        return static_cast<int>(msg.wParam);
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << "\n";
        return -1;
    }
}