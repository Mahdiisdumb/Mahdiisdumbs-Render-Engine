#include <windows.h>
#include "DX12Renderer.h"
#include "ImguiWrapper.h"
#include "imgui/backends/imgui_impl_win32.h"
#include <imgui/imgui.h>

// Forward-declare the Win32 WndProc handler from the ImGui Win32 backend (header keeps it inside a #if 0 block)
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
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
    MessageBoxA(nullptr, "The application crashed. A crash dump was written to crash.dmp", "Crash", MB_OK | MB_ICONERROR); // dumping of the fucking crash dump wow am i really an idiot?
    return EXCEPTION_EXECUTE_HANDLER;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // Forward messages to ImGui only when UI mode is enabled and ImGui is initialized
    extern bool g_uiMode;
    if (ImGui::GetCurrentContext() && g_uiMode) {
        if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
            return TRUE;
    }
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
        // Ensure ImGui global exists
        g_uiMode = false;
        // Global UI interaction toggle. When true, ImGui handles input and scene is not controlled.
        static bool g_uiMode_local = false;
        // expose to ImGuiWrapper/global via external linkage in this translation unit
        g_uiMode = g_uiMode_local;
        HINSTANCE hInstance = GetModuleHandle(nullptr);

        // Register window class
        WNDCLASSW wc = {};
        wc.lpfnWndProc = WndProc;
        wc.hInstance = hInstance;
		wc.lpszClassName = L"MahdiisdumbsRE"; // hey look its the render engine name in the window class name how original 🥀🥀🥀😭😭😭
        RegisterClassW(&wc);

        // Create window
        //its just four errors what the fuck is wrong with me 😭😭
        //now i understand its not the compiler being mean its me being an absolute IDIOT!e
        HWND hwnd = CreateWindowExW( //strange they connect meaning the argument is probably invalid and yea it was invalid i fixed it but i need 4 more arguments probably what comes afdter W and H
            //the thing is int is incompatible with HWND so lets see the reason
            0,
            wc.lpszClassName,
            L"Mahdiisdumbs Render Engine",
// commented out cause why not            RegisterClassW(&wc), // its somthing like this
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            800,//normal
            600,//normal
           //down here is the what ive fucking needed the nullptrs
            nullptr,
            nullptr,
            hInstance, //i hope this thing doesnt break it again
            nullptr
            //i need somthing here i just dont know ill read some docs
            /*err cascadings so thats the problem*/); //no more same errors but syntax erorr with ) weird i mean weird maybe because im dumbass
		//every error fixed but "syntax error: ')'" is it HIGH?! or am i that much of an IMBECILE that i cant even write a function call without syntax errors? ✌️🥀
        // never mind i got jumpscared by my compilation completed sound (scooby doos laugh but retarded)
        //now theres too few args in the call and it doesnt take 8 args are we serious 😭😭
        //just learnt we needed 12 args WTF am i gonna get 12 args from?!
		//"C2660 will occur if a derived class hides a function." quick question microsoft but theres no hidden function or class so why the fuck is this happening
        //"C2660 can occur if you define a new operator in a template class, but where the new operator creates an object whose type is other than the enclosing type." now this is my problem!
		//AI is for loosers ive been using it so thats hypocritical of me but now im using my brain finally 🥀🥀🥀✌️✌️😭😭😭
        //theoritically what would happen if i comment all of this and nevermind because it returns unidentified 😭
        /*// C2660.cpp
void func( int, int ) {}

int main() {
   func( 1 );   // C2660 func( int ) not declared
   func( 1, 0 );   // OK
} ive found a lead and my four missing arguments from the documentation ive read says im misisng  int       nWidth,
  int       nHeight,
  HWND      hWndParent,
  HMENU     hMenu,
  HINSTANCE hInstance,
  LPVOID    lpParam ehhhh ill try to equip those */
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

                // Toggle UI interaction mode with 'U' key (press to toggle)
                static bool prevToggle = false;
                bool curToggle = (GetAsyncKeyState('U') & 0x8000) != 0;
                if (curToggle && !prevToggle) {
                    g_uiMode = !g_uiMode;
                    std::cout << "UI mode " << (g_uiMode ? "ENABLED" : "DISABLED") << std::endl;
                }
                prevToggle = curToggle;

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

                // When UI mode is enabled, skip scene interaction (camera updates). Still render scene.
                if (!g_uiMode) renderer.UpdateCamera(1.0f / 60.0f);
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