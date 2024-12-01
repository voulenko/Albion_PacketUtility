#include "ImGuiRenderer.h"

// Our state
bool show_demo_window = true;
bool show_debug_window = true;
bool ImGuiRenderer::done = false; // Теперь это переменная доступна в других файлах

void ImGuiRenderer::render() {
    // Показ демо окна
    if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
    }

    // Создание собственного окна
    {

        ImGui::Checkbox("Demo Window", &show_demo_window);


        
    }

    // Окно статистики
    if (show_debug_window) {

        
    }
}


void ImGuiRenderer::doRender() {
    std::list<int> numbers;
    // Poll and handle messages (inputs, window resize, etc.)
    MSG msg;
    while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
        if (msg.message == WM_QUIT) {
            done = true; // Устанавливаем флаг для выхода
            return; // Выход из функции, чтобы не продолжать рендеринг
        }
    }

    // Проверяем флаг завершения
    if (done) {
        return; // Если done == true, просто выходим
    }

    // Обработка состояния окна
    if (imguiManager->g_SwapChainOccluded && imguiManager->g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED) {
        ::Sleep(10);
        return; // Выход из функции, если окно скрыто
    }
    imguiManager->g_SwapChainOccluded = false;

    // Обработка изменения размера окна
    if (imguiManager->g_ResizeWidth != 0 && imguiManager->g_ResizeHeight != 0) {
        imguiManager->CleanupRenderTarget();
        imguiManager->g_pSwapChain->ResizeBuffers(0, imguiManager->g_ResizeWidth, imguiManager->g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
        imguiManager->g_ResizeWidth = imguiManager->g_ResizeHeight = 0;
        imguiManager->CreateRenderTarget();
    }

    // Начало нового кадра ImGui
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    render();


    // Рендеринг
    ImGui::Render();
    const float clear_color_with_alpha[4] = { 0,0,0,0 };
    ImGuiManager::g_pd3dDeviceContext->OMSetRenderTargets(1, &ImGuiManager::g_mainRenderTargetView, nullptr);
    ImGuiManager::g_pd3dDeviceContext->ClearRenderTargetView(ImGuiManager::g_mainRenderTargetView, clear_color_with_alpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // Представление
    HRESULT hr = ImGuiManager::g_pSwapChain->Present(1, 0);
    ImGuiManager::g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
}