#pragma once

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include "ImGuiRenderer.h"


class ImGuiManager
{
public:
    // DirectX global variables
    static ID3D11Device* g_pd3dDevice;
    static ID3D11DeviceContext* g_pd3dDeviceContext;
    static IDXGISwapChain* g_pSwapChain;
    static bool g_SwapChainOccluded;
    static UINT g_ResizeWidth, g_ResizeHeight;
    static ID3D11RenderTargetView* g_mainRenderTargetView;
    ImGuiManager();
    ~ImGuiManager();
    // Function declarations
    bool CreateDeviceD3D(HWND hWnd);
    void CleanupDeviceD3D();
    void CreateRenderTarget();
    void CleanupRenderTarget();

    static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    ImGuiIO io;
    // Реализация WndProc

};
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern ImGuiManager* imguiManager;
