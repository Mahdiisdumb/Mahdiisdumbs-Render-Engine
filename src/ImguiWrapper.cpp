#include "ImguiWrapper.h"

#ifdef USE_IMGUI
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"
#include <wrl.h>

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

void ImGuiWrapper::BeginWindow(const char* title) { ImGui::Begin(title); }
void ImGuiWrapper::Text(const char* text) { ImGui::Text("%s", text); }
void ImGuiWrapper::EndWindow() { ImGui::End(); }

#else

bool ImGuiWrapper::Init(HWND, ID3D12Device*, ID3D12CommandQueue*, DXGI_FORMAT, int) { return false; }
void ImGuiWrapper::NewFrame() {}
void ImGuiWrapper::Render(ID3D12GraphicsCommandList*) {}
void ImGuiWrapper::Shutdown() {}

#endif
