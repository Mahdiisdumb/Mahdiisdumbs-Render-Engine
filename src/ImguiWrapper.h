#pragma once
#include <Windows.h>
#include <d3d12.h>

class ImGuiWrapper {
public:
    // Initialize ImGui renderer (optional). Returns true on success or if ImGui is not present (no-op).
    static bool Init(HWND hwnd, ID3D12Device* device, ID3D12CommandQueue* queue, DXGI_FORMAT rtvFormat, int numFramesInFlight = 2);
    static void NewFrame();
    static void Render(ID3D12GraphicsCommandList* cmdList);
    static void Shutdown();
    // Simple UI helpers used by the engine to avoid direct dependency on ImGui headers
    static void BeginWindow(const char* title) { /* no-op if ImGui not available */ }
    static void Text(const char* text) { /* no-op */ }
    static void EndWindow() { /* no-op */ }
};
