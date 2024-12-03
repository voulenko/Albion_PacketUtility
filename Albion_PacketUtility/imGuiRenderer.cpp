#include "ImGuiRenderer.h"
#include <vector>
#include <iostream>
#include <chrono>
#include <ctime>
#include <string>
// Our state
bool show_demo_window = true;
bool show_debug_window = true;
bool ImGuiRenderer::done = false; // Теперь это переменная доступна в других файлах
std::vector<data> ImGuiRenderer::test; //  Определяем так как переменная статическая
/// Универсальный метод для преобразования одиночных значений
template <typename T>
const char* to_char(const T& value) {
    static char buffer[20];  // Буфер для хранения результата
    if constexpr (std::is_same<T, int>::value) {
        sprintf_s(buffer, sizeof(buffer), "%d", value);
    }
    else if constexpr (std::is_same<T, float>::value) {
        sprintf_s(buffer, sizeof(buffer), "%.2f", value);
    }
    else if constexpr (std::is_same<T, DWORD_PTR>::value) {
        sprintf_s(buffer, sizeof(buffer), "0x%lX", static_cast<unsigned long long>(value));
    }
    return buffer;
}

void ImGuiRenderer::render() {
    // Показ демо окна
    if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
    }

    // Создание собственного окна
    {
        if(ImGui::Button("TEST")){

            // Получаем текущее время
            auto now = std::chrono::system_clock::now();
            std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

            // Преобразуем в строку
            char timeBuffer[100];
            std::tm timeInfo;
            // Используем localtime_s для безопасного преобразования времени
            localtime_s(&timeInfo, &currentTime);  // safe localtime_s instead of unsafe localtime
            std::strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", &timeInfo);
            //test.push_back(data{ 1, "event", timeBuffer, "NULL"});
            
        }
        ImGui::Text(to_char((int)test.size()));
        ImGui::Checkbox("Demo Window", &show_demo_window);
        
        ImGuiTableFlags flags =
            ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_ScrollY;
        if (ImGui::BeginTable("mobs", 4, flags, ImVec2(0.0f, 400), 0.0f)) // Изменяем количество столбцов на 1
        {

            ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed, 100.0f); // 100.0f - фиксированная ширина
            ImGui::TableSetupColumn("Packet", ImGuiTableColumnFlags_WidthFixed, 50.0f); // 150.0f - фиксированная ширина
            ImGui::TableSetupColumn("Code", ImGuiTableColumnFlags_WidthFixed, 30.0f); // 120.0f - фиксированная ширина
            ImGui::TableSetupColumn("Parameters", ImGuiTableColumnFlags_WidthFixed, 300.0f); // 200.0f - фиксированная ширина
            ImGui::TableSetupScrollFreeze(0, 1); // Make row always visible
            ImGui::TableHeadersRow();
            

            for (int i = 0; i < test.size(); ++i) {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::Text(test[i].time.c_str());  // Используем .c_str() для передачи C-строки

                ImGui::TableSetColumnIndex(1);
                ImGui::Text(test[i].packet.c_str());

                ImGui::TableSetColumnIndex(2);
                ImGui::Text(std::to_string(test[i].code).c_str());  // Преобразуем целое число в строку

                ImGui::TableSetColumnIndex(3);
                ImGui::PushID(i);
                if (ImGui::CollapsingHeader(to_char((int)test[i].parameters.size()), ImGuiTreeNodeFlags_AllowItemOverlap)) {
                    for (const auto& param : test[i].parameters) {
                        DeserializedValue params = param.second;
                        ImGui::Text("[%d] [%s:%d] | %s",
                            static_cast<int>(param.first),
                            params.getTypeStr(),
                            static_cast<int>(params.type),
                            params.getValueStr().c_str());
                    }
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
        
        
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