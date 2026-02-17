#include <windows.h>
#include "DX12Renderer.h"
#include "ImguiWrapper.h"
#include "imgui/backends/imgui_impl_win32.h"
#include <iostream>
#include <DbgHelp.h>
#define USE_ASSIMP

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
#ifdef USE_IMGUI
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return TRUE;
#endif
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
        if (!ImGuiWrapper::Init(hwnd, renderer.GetDevice(), renderer.GetCommandQueue(), DXGI_FORMAT_R8G8B8A8_UNORM)) {
            std::cerr << "ImGui initialization failed or disabled; UI will be unavailable.\n";
        }

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

               ImGuiWrapper::BeginWindow("Help");
               ImGuiWrapper::Text("F1 For help");
               ImGuiWrapper::EndWindow();

                // Toggle importer window with Ctrl+I
                static bool prevCtrlI = false;
                bool curCtrlI = (GetAsyncKeyState('I') & 0x8000) != 0 && ctrl;
                if (curCtrlI && !prevCtrlI) {
                    ImGuiWrapper::g_showImportWindow = !ImGuiWrapper::g_showImportWindow;
                }
                prevCtrlI = curCtrlI;

                // F1 -> Toggle controls window
                static bool prevF1 = false;
                bool curF1 = (GetAsyncKeyState(VK_F1) & 0x8000) != 0;
                if (curF1 && !prevF1) {
                    ImGuiWrapper::g_showControlsWindow = !ImGuiWrapper::g_showControlsWindow;
                }
                prevF1 = curF1;

                // Build ImGui windows for this frame BEFORE rendering so DX12Renderer::Render can submit drawdata
                ImGuiWrapper::RenderImportUI(&renderer);
                ImGuiWrapper::RenderColorPicker();
                ImGuiWrapper::RenderControlsWindow();

                renderer.UpdateCamera(1.0f / 60.0f);
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