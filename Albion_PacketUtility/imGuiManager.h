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
            // ������������ � ����� �������
            // ������� ����� � ������ ���� �������������
            dwStyle &= ~WS_OVERLAPPEDWINDOW;
            dwExStyle |= WS_EX_TOPMOST | WS_EX_LAYERED;

            // ������������� �����
            SetWindowLongPtr(hwnd, GWL_STYLE, dwStyle);
            SetWindowLongPtr(hwnd, GWL_EXSTYLE, dwExStyle);

            // ������������� ����� ��������� (�� ���� �����)
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED);

            // ���� ���������� ���������� ��� ������ ����
            SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

        }
        else {
            // ������������ � ������� ������� �����
            // ��������������� ����� ����
            dwStyle |= WS_OVERLAPPEDWINDOW;
            dwExStyle &= ~(WS_EX_TOPMOST | WS_EX_LAYERED);

            // ��������� ��������� �������, ������� ����� ��� ��������� �������
            dwStyle &= ~WS_THICKFRAME;   // ������� �����, ����������� �������� ������
            dwStyle &= ~WS_SIZEBOX;      // ������� �����, ����������� �������� ������ ����� ����

            dwStyle &= ~WS_CAPTION; // ������� ����������� ���������

            // ������������� �����
            SetWindowLongPtr(hwnd, GWL_STYLE, dwStyle);
            SetWindowLongPtr(hwnd, GWL_EXSTYLE, dwExStyle);

            // ������������� ����� ��������� (�� ���������)
            SetWindowPos(hwnd, HWND_NOTOPMOST, 100, 100, 800, 740, SWP_FRAMECHANGED);

            // ������� ������������ ��� ������ ����
            SetLayeredWindowAttributes(hwnd, 0, 0, 0);
        }

        // ��������� ����
        SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
    }

    static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    ImGuiIO io;
    // ���������� WndProc

};
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern ImGuiManager* imguiManager;
