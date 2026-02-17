#define USE_IMGUI
#include "ImguiWrapper.h"// Storage for header-declared static flags (always present regardless of USE_IMGUI)
#include "DX12Renderer.h"
bool ImGuiWrapper::g_showImportWindow = false;
bool ImGuiWrapper::g_showColorPicker = false;
bool ImGuiWrapper::g_showControlsWindow = false;

#ifdef USE_IMGUI
#include "imgui/imgui.h"
#include <wrl.h>

// Include implementation files here to build single-translation-unit variant used by this project.
#include "imgui/imgui.cpp"
#include "imgui/imgui_draw.cpp"
#include "imgui/imgui_tables.cpp"
#include "imgui/imgui_widgets.cpp"
#include "imgui/backends/imgui_impl_win32.cpp"
#include "imgui/backends/imgui_impl_dx12.cpp"

using Microsoft::WRL::ComPtr;

static ComPtr<ID3D12DescriptorHeap> s_srvHeap;

bool ImGuiWrapper::Init(HWND hwnd, ID3D12Device* device, ID3D12CommandQueue* queue, DXGI_FORMAT rtvFormat, int numFramesInFlight) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NumDescriptors = 1;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ComPtr<ID3D12DescriptorHeap> heap;
    if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap)))) return false;
    s_srvHeap = heap;

    ImGui_ImplWin32_Init(hwnd);

    ImGui_ImplDX12_InitInfo init_info;
    init_info.Device = device;
    init_info.CommandQueue = queue;
    init_info.NumFramesInFlight = numFramesInFlight;
    init_info.RTVFormat = rtvFormat;
    init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;
    init_info.SrvDescriptorHeap = heap.Get();
    init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu) {
        ID3D12DescriptorHeap* h = info->SrvDescriptorHeap;
        *out_cpu = h->GetCPUDescriptorHandleForHeapStart();
        *out_gpu = h->GetGPUDescriptorHandleForHeapStart();
    };
    init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE) {};

    if (!ImGui_ImplDX12_Init(&init_info)) return false;
    return true;
}

void ImGuiWrapper::RenderControlsWindow() {
    if (!g_showControlsWindow) return;
    ImGui::Begin("Controls", &g_showControlsWindow);
    ImGui::Text("Camera Movement:");
    ImGui::Text("  W/A/S/D - Move");
    ImGui::Text("  RMB + Move Mouse - Look Around");
    ImGui::Text("  Shift - Sprint");
    ImGui::Separator();
    ImGui::Text("Hotkeys:");
    ImGui::Text("  F1 - Toggle this controls window");
    ImGui::Text("  Ctrl+O - Open Model");
    ImGui::Text("  Ctrl+T - Open Texture");
    ImGui::Text("  Ctrl+I - Toggle Importer window");
    ImGui::Text("  Ctrl+Alt+C - Toggle Color Picker");
    ImGui::End();
}

void ImGuiWrapper::NewFrame() {
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void ImGuiWrapper::Render(ID3D12GraphicsCommandList* cmdList) {
    ImGui::Render();
    if (s_srvHeap) {
        ID3D12DescriptorHeap* heaps[] = { s_srvHeap.Get() };
        cmdList->SetDescriptorHeaps(_countof(heaps), heaps);
    }
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList);
}

void ImGuiWrapper::Shutdown() {
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiWrapper::BeginWindow(const char* title) { 
#ifdef USE_IMGUI
    ImGui::Begin(title); 
#endif
}
void ImGuiWrapper::Text(const char* text) {
#ifdef USE_IMGUI
    ImGui::Text("%s", text);
#endif
}
void ImGuiWrapper::EndWindow() {
#ifdef USE_IMGUI
    ImGui::End();
#endif
}

// High level UI helpers (definitions)
static float g_color[4] = {1.0f, 0.75f, 0.8f, 1.0f};

void ImGuiWrapper::RenderImportUI(DX12Renderer* renderer) {
    if (!g_showImportWindow) return;
    ImGui::Begin("Importer", &g_showImportWindow);

#ifdef USE_ASSIMP
    ImGui::Text("Assimp: Enabled");
#else
    ImGui::Text("Assimp: Disabled (define USE_ASSIMP to enable)");
#endif

    ImGui::Text("Hotkeys: Ctrl+O = Open Model, Ctrl+T = Open Texture, Ctrl+Alt+C = Toggle Color Picker");

    if (ImGui::Button("Load Model (Ctrl+O)")) {
        // Show Win32 open file dialog
        OPENFILENAMEA ofn;
        CHAR szFile[MAX_PATH] = {0};
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = GetActiveWindow();
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = "Model Files\0*.fbx;*.obj\0All\0*.*\0";
        ofn.Flags = OFN_FILEMUSTEXIST;
        if (GetOpenFileNameA(&ofn)) {
            if (renderer) renderer->LoadModel(szFile);
        }
    }

    if (ImGui::Button("Load Texture (Ctrl+T)")) {
        OPENFILENAMEA ofn;
        CHAR szFile[MAX_PATH] = {0};
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = GetActiveWindow();
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = "Image Files\0*.png;*.jpg;*.jpeg;*.dds\0All\0*.*\0";
        ofn.Flags = OFN_FILEMUSTEXIST;
        if (GetOpenFileNameA(&ofn)) {
            if (renderer) renderer->LoadTexture(szFile);
        }
    }

    ImGui::End();
}

void ImGuiWrapper::RenderColorPicker() {
    if (!g_showColorPicker) return;
    ImGui::Begin("Color Picker", &g_showColorPicker);
    ImGui::ColorEdit4("Color", g_color);
    ImGui::End();
}

#else

bool ImGuiWrapper::Init(HWND, ID3D12Device*, ID3D12CommandQueue*, DXGI_FORMAT, int) { return false; }
void ImGuiWrapper::NewFrame() {}
void ImGuiWrapper::Render(ID3D12GraphicsCommandList*) {}
void ImGuiWrapper::Shutdown() {}

// Non-UI (no ImGui) stubs
void ImGuiWrapper::RenderImportUI(DX12Renderer* renderer) { (void)renderer; }
void ImGuiWrapper::RenderColorPicker() {}
void ImGuiWrapper::RenderControlsWindow() { (void)g_showControlsWindow; }

#endif
