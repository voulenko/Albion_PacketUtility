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
ExampleAppLog ImGuiRenderer::log;
bool ImGuiRenderer::capture;

static int redact = -1;

std::vector<layoutPacket> ImGuiRenderer::packetsSettings;

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

    bool open = true;

    if (ImGuiManager::Overlay) {
        ImVec2 fixedSize(800, 740);
        // Создание собственного окна
        ImGui::SetNextWindowSize(fixedSize, ImGuiCond_Always);
        ImGui::Begin("Debugger", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    }
    else {
        ImVec2 fixedSize(800, 740);
        // Устанавливаем размер окна
        ImGui::SetNextWindowSize(fixedSize, ImGuiCond_Always);
        ImGui::Begin("Debugger", &open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);
        // Устанавливаем позицию окна
        ImGui::SetWindowPos(ImVec2(0, 0));


        // Включаем функцию для перетаскивания окна
        DragWindow(ImGuiManager::hwnd);
        //ImGui::End();  // Завершаем окно ImGui
    }

    if (!open) {
        PostMessage(ImGuiManager::hwnd, WM_CLOSE, 0, 0); // Закрытие окна
    }

    ImGui::BeginGroup();

    ImGui::BeginChild("Settings", ImVec2(180, 500), true);
    {
        ImGui::SeparatorText("PACKET DISPLAY");

        ImGui::Checkbox("Capture", &capture);
        ImGui::SameLine();
        if (ImGuiManager::Overlay) {
            if (ImGui::Button("To Window")) {
                ImGuiManager::Overlay = false;
                ImGuiManager::SwitchWindowMode(false);
            }
        }
        else {
            if (ImGui::Button("To Overlay")) {
                ImGuiManager::Overlay = true;
                ImGuiManager::SwitchWindowMode(true);
            }
        }

        if (ImGui::Button("Clear")) {
            test.clear();
        }
        ImGui::SameLine();
        ImGui::Text("Count: ");
        ImGui::SameLine();
        ImGui::Text(to_char((int)test.size()));
        ImGui::Checkbox("Demo Window", &show_demo_window);

        if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl) ||
            ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift) ||
            ImGui::IsKeyDown(ImGuiKey_LeftAlt) || ImGui::IsKeyDown(ImGuiKey_RightAlt)) {
            ImGui::SetKeyboardFocusHere();
        }

        ImGui::Separator();

        if (ImGui::Button("Load"))
        {
            loadPacketsFromFile("packets.json", packetsSettings);
        }
        ImGui::SameLine();
        if (ImGui::Button("Save"))
        {
            savePacketsToFile("packets.json", packetsSettings);
        }
        if (ImGui::CollapsingHeader("Event", ImGuiTreeNodeFlags_AllowItemOverlap))
        {
            static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
            for (int index = 0; index < 500; index++) {
                ImGui::PushID(index);
                ImGuiTreeNodeFlags node_flags = base_flags;
                if (index == redact) node_flags |= ImGuiTreeNodeFlags_Selected;
                    
                    
                node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                // Доступ к полям структуры layoutPacket
                if(packetsSettings[index].active)
                {
                    if(packetsSettings[index].header != "") ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                    else ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                }
                else
                {
                    if (packetsSettings[index].header != "") ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
                    else ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
                }
                if(packetsSettings[index].header != "") ImGui::TreeNodeEx((void*)(intptr_t)packetsSettings[index].id, node_flags, "[%d] - %s", packetsSettings[index].id, packetsSettings[index].header.c_str());
                else ImGui::TreeNodeEx((void*)(intptr_t)packetsSettings[index].id, node_flags, "[%d] none", packetsSettings[index].id);
                ImGui::PopStyleColor();
                
                if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                {

                    std::string text;
                    if ((int)packetsSettings[index].type == 4) text = "Event";
                    else if ((int)packetsSettings[index].type == 3) text = "Reposnse";
                    else if ((int)packetsSettings[index].type == 2) text = "Request";
                    log.AddLog("selected index %d and %s\n", index, text.c_str());
                    redact = index;
                }

                if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered()) {
                    //    // Здесь выводится информация о двойном клике
                    if (packetsSettings[index].active) packetsSettings[index].active = false;
                    else packetsSettings[index].active = true;
                    //log.AddLog("TEST CLICK %d\n", packet.id);
                }
                ImGui::PopID();
            }
        }
        
        if (ImGui::CollapsingHeader("Response", ImGuiTreeNodeFlags_AllowItemOverlap))
        {
            static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
            for (int index = 500; index < 1000; index++) {
                ImGui::PushID(index);
                ImGuiTreeNodeFlags node_flags = base_flags;
                if (index == redact) node_flags |= ImGuiTreeNodeFlags_Selected;


                node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                // Доступ к полям структуры layoutPacket
                if (packetsSettings[index].active)
                {
                    if (packetsSettings[index].header != "") ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                    else ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                }
                else
                {
                    if (packetsSettings[index].header != "") ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
                    else ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
                }
                if (packetsSettings[index].header != "") ImGui::TreeNodeEx((void*)(intptr_t)packetsSettings[index].id, node_flags, "[%d] - %s", packetsSettings[index].id, packetsSettings[index].header.c_str());
                else ImGui::TreeNodeEx((void*)(intptr_t)packetsSettings[index].id, node_flags, "[%d] none", packetsSettings[index].id);
                ImGui::PopStyleColor();

                if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                {

                    std::string text;
                    if ((int)packetsSettings[index].type == 4) text = "Event";
                    else if ((int)packetsSettings[index].type == 3) text = "Reposnse";
                    else if ((int)packetsSettings[index].type == 2) text = "Request";
                    log.AddLog("selected index %d and %s\n", index, text.c_str());
                    redact = index;
                }

                if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered()) {
                    //    // Здесь выводится информация о двойном клике
                    if (packetsSettings[index].active) packetsSettings[index].active = false;
                    else packetsSettings[index].active = true;
                    //log.AddLog("TEST CLICK %d\n", packet.id);
                }
                ImGui::PopID();
            }
            
        }
        
        if (ImGui::CollapsingHeader("Request", ImGuiTreeNodeFlags_AllowItemOverlap))
        {
            static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
            for (int index = 1000; index < packetsSettings.size(); index++) {
                ImGui::PushID(index);
                ImGuiTreeNodeFlags node_flags = base_flags;
                if (index == redact) node_flags |= ImGuiTreeNodeFlags_Selected;


                node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                // Доступ к полям структуры layoutPacket
                if (packetsSettings[index].active)
                {
                    if (packetsSettings[index].header != "") ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                    else ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                }
                else
                {
                    if (packetsSettings[index].header != "") ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
                    else ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
                }
                if (packetsSettings[index].header != "") ImGui::TreeNodeEx((void*)(intptr_t)packetsSettings[index].id, node_flags, "[%d] - %s", packetsSettings[index].id, packetsSettings[index].header.c_str());
                else ImGui::TreeNodeEx((void*)(intptr_t)packetsSettings[index].id, node_flags, "[%d] none", packetsSettings[index].id);
                ImGui::PopStyleColor();

                if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                {

                    std::string text;
                    if ((int)packetsSettings[index].type == 4) text = "Event";
                    else if ((int)packetsSettings[index].type == 3) text = "Reposnse";
                    else if ((int)packetsSettings[index].type == 2) text = "Request";
                    log.AddLog("selected index %d and %s\n", index, text.c_str());
                    redact = index;
                }

                if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered()) {
                    //    // Здесь выводится информация о двойном клике
                    if (packetsSettings[index].active) packetsSettings[index].active = false;
                    else packetsSettings[index].active = true;
                    //log.AddLog("TEST CLICK %d\n", packet.id);
                }
                ImGui::PopID();
            }
        }


        ImGui::EndChild();
    }

    ImGui::BeginChild("Layout settings", ImVec2(180, 200), true);
    {
        if (redact != -1) {

            std::string text;
            if ((int)packetsSettings[redact].type == 4) text = "Event";
            else if ((int)packetsSettings[redact].type == 3) text = "Reposnse";
            else if ((int)packetsSettings[redact].type == 2) text = "Request";
            
            ImGui::Text("[type] %s", text.c_str());
            ImGui::Text("");

            ImGui::Text("[id] %d", packetsSettings[redact].id);

            ImGui::Text("[header]");
            ImGui::SameLine();

            constexpr size_t BUFFER_SIZE_H = 12; // Размер буфера
            char buffer_H[BUFFER_SIZE_H];

            // Копируем строку из header в buffer
            strncpy_s(buffer_H, packetsSettings[redact].header.c_str(), BUFFER_SIZE_H);
            buffer_H[BUFFER_SIZE_H - 1] = '\0'; // Обеспечиваем нуль-терминированность

            ImGui::PushItemWidth(100.0f);
            if (ImGui::InputText(" ", buffer_H, BUFFER_SIZE_H, ImGuiInputTextFlags_CharsUppercase)) {
                // Если текст изменился, сохраняем его обратно в packetsSettings
                packetsSettings[redact].header = buffer_H;
            }
            ImGui::PopItemWidth();
            ImGui::Text("[desc] \\/");

            constexpr size_t BUFFER_SIZE = 1024; // Укажите нужный размер
            char buffer[BUFFER_SIZE];
            strncpy_s(buffer, packetsSettings[redact].description.c_str(), BUFFER_SIZE);
            buffer[BUFFER_SIZE - 1] = '\0'; // Обеспечиваем нуль-терминированность
            static ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
            ImGui::PushFont(imguiManager->customFont);
            if (ImGui::InputTextMultiline("##source", buffer, IM_ARRAYSIZE(buffer), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 5), flags)) {

                packetsSettings[redact].description = buffer;
            }
            ImGui::PopFont();
            // Если нужно сохранить изменения обратно:
            

            ImGui::Text("[active] %s", packetsSettings[redact].active ? "true" : "false");

        }




        ImGui::EndChild();
    }

    ImGui::EndGroup();
    ImGui::SameLine();
    // Начало окна с запретом изменения размеров и сворачивания
    //ImGui::SetNextWindowSize(fixedSize, ImGuiCond_Always);

    ImGui::BeginGroup();

    ImGui::BeginChild("Packets list", ImVec2(600, 500), true);
    {   
        ImGuiTableFlags flags =
            ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_ScrollY;
        if (ImGui::BeginTable("mobs", 4, flags, ImVec2(0.0f, 480), 0.0f)) // Изменяем количество столбцов на 1
        {

            ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed, 100.0f); // 100.0f - фиксированная ширина
            ImGui::TableSetupColumn("Packet", ImGuiTableColumnFlags_WidthFixed, 50.0f); // 150.0f - фиксированная ширина
            ImGui::TableSetupColumn("Code", ImGuiTableColumnFlags_WidthFixed, 30.0f); // 120.0f - фиксированная ширина
            ImGui::TableSetupColumn("Parameters", ImGuiTableColumnFlags_WidthFixed, 300.0f); // 200.0f - фиксированная ширина
            ImGui::TableSetupScrollFreeze(0, 1); // Make row always visible
            ImGui::TableHeadersRow();
            

            for (int i = 0; i < test.size(); ++i) {

                if (test[i].packet == "Event")
                    if (!packetsSettings[test[i].code].active)
                        continue;
                if (test[i].packet == "Response")
                    if (!packetsSettings[test[i].code+500].active)
                        continue;
                if (test[i].packet == "Request")
                    if (!packetsSettings[test[i].code+1000].active)
                        continue;
                
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::Text(test[i].time.c_str());  // Используем .c_str() для передачи C-строки

                ImGui::TableSetColumnIndex(1);
                ImGui::Text(test[i].packet.c_str());

                ImGui::TableSetColumnIndex(2);
                std::string headerPacket = to_char((int)test[i].parameters.size());
                if (test[i].packet == "Event") {
                    if (packetsSettings[test[i].code].header != "") {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.6f, 1.0f));
                        ImGui::Text(std::to_string(test[i].code).c_str());  // Преобразуем целое число в строку
                        ImGui::PopStyleColor();
                        headerPacket += " " + packetsSettings[test[i].code].header;
                    }
                    else {
                        ImGui::Text(std::to_string(test[i].code).c_str());  // Преобразуем целое число в строку
                    }
                }
                else if (test[i].packet == "Response") {
                    if (packetsSettings[test[i].code+500].header != "") {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.6f, 1.0f));
                        ImGui::Text(std::to_string(test[i].code).c_str());  // Преобразуем целое число в строку
                        ImGui::PopStyleColor();
                        headerPacket += " " + packetsSettings[test[i].code+500].header;
                    }
                    else {
                        ImGui::Text(std::to_string(test[i].code).c_str());  // Преобразуем целое число в строку
                    }
                }
                else if (test[i].packet == "Request") {
                    if (packetsSettings[test[i].code + 1000].header != "") {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.6f, 1.0f));
                        ImGui::Text(std::to_string(test[i].code).c_str());  // Преобразуем целое число в строку
                        ImGui::PopStyleColor();
                        headerPacket += " " + packetsSettings[test[i].code + 1000].header;
                    }
                    else {
                        ImGui::Text(std::to_string(test[i].code).c_str());  // Преобразуем целое число в строку
                    }
                }
                
                ImGui::TableSetColumnIndex(3);
                ImGui::PushID(i);
                if (ImGui::CollapsingHeader(headerPacket.c_str(), ImGuiTreeNodeFlags_AllowItemOverlap)) {

                    for (const auto& param : test[i].parameters) {
                        DeserializedValue params = param.second;
                        if (params.type == Type::ByteArray) {
                            ImGui::Text("[%d] [%s:%d] | %s",
                                static_cast<int>(param.first),
                                params.getTypeStr(),
                                static_cast<int>(params.type),
                                params.getValueStr().c_str());


                            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.0f, 1.0f, 0.6f, 0.5f));
                            ImGui::SameLine();
                            ImGui::PushID(param.first);
                            if (ImGui::CollapsingHeader("", ImGuiTreeNodeFlags_AllowItemOverlap)) {
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.6f, 1.0f));
                                ImGui::Text("[%s]", params.getTypeStr());

                                for (int i = 0; i < std::static_pointer_cast<std::vector<uint8_t>>(params.value)->size(); ++i)
                                    ImGui::Text("[%d] %02X : %d", i, (*std::static_pointer_cast<std::vector<uint8_t>>(params.value))[i], (*std::static_pointer_cast<std::vector<uint8_t>>(params.value))[i]);

                                ImGui::PopStyleColor();
                            }
                            ImGui::PopID();
                            ImGui::PopStyleColor();
                            ImGui::PopStyleVar(); // Восстанавливаем отступы

                        }
                        else if (params.type == Type::Array) {
                            ImGui::Text("[%d] [%s:%d] | %s",
                                static_cast<int>(param.first),
                                params.getTypeStr(),
                                static_cast<int>(params.type),
                                params.getValueStr().c_str());

                            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.0f, 1.0f, 0.6f, 0.5f));
                            ImGui::SameLine();
                            ImGui::PushID(param.first);

                            DeserializedValue& param1 = *std::static_pointer_cast<DeserializedValue>(params.value);
                            if (ImGui::CollapsingHeader("", ImGuiTreeNodeFlags_AllowItemOverlap)) {
                                
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.6f, 1.0f));
                                ImGui::Text("[%s]", param1.getTypeStr());

                                switch (param1.type) 
                                {

                                    case Type::String:
                                        for (int i = 0; i < std::static_pointer_cast<std::vector<std::string>>(param1.value)->size(); ++i)
                                            ImGui::Text("[%d] %s", i, (*std::static_pointer_cast<std::vector<std::string>>(param1.value))[i].c_str());
                                        break;
                                    case Type::Integer:
                                        for (int i = 0; i < std::static_pointer_cast<std::vector<int32_t>>(param1.value)->size(); ++i)
                                            ImGui::Text("[%d] %d", i, (*std::static_pointer_cast<std::vector<int32_t>>(param1.value))[i]);
                                        break;
                                    case Type::Long:
                                        for (int i = 0; i < std::static_pointer_cast<std::vector<int64_t>>(param1.value)->size(); ++i)
                                            ImGui::Text("[%d] %lld", i, (*std::static_pointer_cast<std::vector<int64_t>>(param1.value))[i]);
                                        break;
                                    case Type::Float:
                                        for (int i = 0; i < std::static_pointer_cast<std::vector<float>>(param1.value)->size(); ++i)
                                            ImGui::Text("[%d] %f", i, (*std::static_pointer_cast<std::vector<float>>(param1.value))[i]);
                                        break;
                                    case Type::Short:
                                        for (int i = 0; i < std::static_pointer_cast<std::vector<int16_t>>(param1.value)->size(); ++i)
                                            ImGui::Text("[%d] %d", i, (*std::static_pointer_cast<std::vector<int16_t>>(param1.value))[i]);
                                        break;
                                    default:
                                        ImGui::Text("-----");
                                    
                                }


                                ImGui::PopStyleColor();
                            }
                            ImGui::PopID();
                            ImGui::PopStyleColor();
                            ImGui::PopStyleVar(); // Восстанавливаем отступы
                        }
                        else {
                            ImGui::Text("[%d] [%s:%d] | %s",
                                static_cast<int>(param.first),
                                params.getTypeStr(),
                                static_cast<int>(params.type),
                                params.getValueStr().c_str());
                        }
                        
                    }
                }
                
                ImGui::PopID();
                
            }
            ImGui::EndTable();
        }
        
        
    }
    ImGui::EndChild();

    //ImGui::SameLine();
    

    // 2е ОКНО

    log.Draw();
    ImGui::EndGroup();
    ImGui::End();
    
    

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

