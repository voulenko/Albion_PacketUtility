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
    static ImFont* customFont;
    static ID3D11Device* g_pd3dDevice;
    static ID3D11DeviceContext* g_pd3dDeviceContext;
    static IDXGISwapChain* g_pSwapChain;
    static bool g_SwapChainOccluded;
    static UINT g_ResizeWidth, g_ResizeHeight;
    static ID3D11RenderTargetView* g_mainRenderTargetView;
    static bool Overlay;
    ImGuiManager();
    ~ImGuiManager();
    // Function declarations
    bool CreateDeviceD3D(HWND hWnd);
    void CleanupDeviceD3D();
    void CreateRenderTarget();
    void CleanupRenderTarget();

    static WNDCLASSEXW wc;
    static HWND hwnd;

    static void SwitchWindowMode(bool isOverlay) {
        DWORD dwStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
        DWORD dwExStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);

        if (isOverlay) {
            // Переключение в режим оверлея
            // Убираем рамки и делаем окно полноэкранным
            dwStyle &= ~WS_OVERLAPPEDWINDOW;
            dwExStyle |= WS_EX_TOPMOST | WS_EX_LAYERED;

            // Устанавливаем флаги
            SetWindowLongPtr(hwnd, GWL_STYLE, dwStyle);
            SetWindowLongPtr(hwnd, GWL_EXSTYLE, dwExStyle);

            // Устанавливаем новое положение (на весь экран)
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED);

            // Окно становится прозрачным для кликов мыши
            SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

        }
        else {
            // Переключение в обычный оконный режим
            // Восстанавливаем стиль окна
            dwStyle |= WS_OVERLAPPEDWINDOW;
            dwExStyle &= ~(WS_EX_TOPMOST | WS_EX_LAYERED);

            // Блокируем изменение размера, убираем флаги для изменения размера
            dwStyle &= ~WS_THICKFRAME;   // Убираем стиль, позволяющий изменять размер
            dwStyle &= ~WS_SIZEBOX;      // Убираем стиль, позволяющий изменять размер через угол

            dwStyle &= ~WS_CAPTION; // Убираем стандартный заголовок

            // Устанавливаем флаги
            SetWindowLongPtr(hwnd, GWL_STYLE, dwStyle);
            SetWindowLongPtr(hwnd, GWL_EXSTYLE, dwExStyle);

            // Устанавливаем новое положение (по умолчанию)
            SetWindowPos(hwnd, HWND_NOTOPMOST, 100, 100, 800, 740, SWP_FRAMECHANGED);

            // Убираем прозрачность для кликов мыши
            SetLayeredWindowAttributes(hwnd, 0, 0, 0);
        }

        // Обновляем окно
        SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
    }

    static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    ImGuiIO io;
    // Реализация WndProc

};
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern ImGuiManager* imguiManager;
