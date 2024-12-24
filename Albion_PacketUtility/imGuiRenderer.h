#pragma once
#include "ImGuiManager.h"
#include <list> 
#include <string>
#include <vector>
#include <Deserializer.h>
#include <Parser.h>

#include <fstream>  
#include <../json.hpp>
using json = nlohmann::json;

struct data {
	std::string time;
	std::string packet;
	int code;
	std::vector<std::pair<uint8_t, DeserializedValue>> parameters;
};

struct layoutPacket
{
    int id;
    std::string header;
    std::string description;
    bool active;
    MessageType type;
};

struct ExampleAppLog
{
    ImGuiTextBuffer     Buf;
    ImGuiTextFilter     Filter;
    ImVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
    bool                AutoScroll;  // Keep scrolling if already at the bottom.

    ExampleAppLog()
    {
        AutoScroll = true;
        Clear();
    }

    void Clear()
    {
        Buf.clear();
        LineOffsets.clear();
        LineOffsets.push_back(0);
    }

    void AddLog(const char* fmt, ...) IM_FMTARGS(2)
    {
        int old_size = Buf.size();
        va_list args;
        va_start(args, fmt);
        Buf.appendfv(fmt, args);
        va_end(args);
        for (int new_size = Buf.size(); old_size < new_size; old_size++)
            if (Buf[old_size] == '\n')
                LineOffsets.push_back(old_size + 1);
    }

    void Draw()
    {


        // Options menu
        if (ImGui::BeginPopup("Options"))
        {
            ImGui::Checkbox("Auto-scroll", &AutoScroll);
            ImGui::EndPopup();
        }

        // Main window
        if (ImGui::Button("Options"))
            ImGui::OpenPopup("Options");
        ImGui::SameLine();
        bool clear = ImGui::Button("Clear");
        ImGui::SameLine();
        /*bool copy = ImGui::Button("Copy");
        ImGui::SameLine();*/
        Filter.Draw("Filter", -100.0f);

        ImGui::Separator();

        if (ImGui::BeginChild("scrolling", ImVec2(600, 150), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
        {
            if (clear)
                Clear();
            /*if (copy)
                ImGui::LogToClipboard();*/

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            const char* buf = Buf.begin();
            const char* buf_end = Buf.end();
            if (Filter.IsActive())
            {
                for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
                {
                    const char* line_start = buf + LineOffsets[line_no];
                    const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                    if (Filter.PassFilter(line_start, line_end))
                        ImGui::TextUnformatted(line_start, line_end);
                }
            }
            else
            {
                ImGuiListClipper clipper;
                clipper.Begin(LineOffsets.Size);
                while (clipper.Step())
                {
                    for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                    {
                        const char* line_start = buf + LineOffsets[line_no];
                        const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                        ImGui::TextUnformatted(line_start, line_end);
                    }
                }
                clipper.End();
            }
            ImGui::PopStyleVar();

            // Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
            // Using a scrollbar or mouse-wheel will take away from the bottom edge.
            if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();

    }
};

// Специализация для сериализации и десериализации layoutPacket в JSON
inline void to_json(nlohmann::json& j, const layoutPacket& packet) {
    j = nlohmann::json{
        {"id", packet.id},
        {"header", packet.header},
        {"description", packet.description},
        {"active", packet.active},
        {"type", static_cast<int>(packet.type)}  // Преобразование MessageType в int, если это перечисление
    };
}

inline void from_json(const nlohmann::json& j, layoutPacket& packet) {
    j.at("id").get_to(packet.id);
    j.at("header").get_to(packet.header);
    j.at("description").get_to(packet.description);
    j.at("active").get_to(packet.active);
    j.at("type").get_to(packet.type);  // Если MessageType - это перечисление, он будет автоматически преобразован
}

class ImGuiRenderer {
public:
	static std::vector<data> test;
    static bool capture;
	static void render();
	static void doRender();
	static bool done;
	static ExampleAppLog log;
    static std::vector<layoutPacket> packetsSettings;
    static void savePacketsToFile(const std::string& filename, const std::vector<layoutPacket>& packets) {
        json j = packets;
        std::ofstream file(filename);
        file << j.dump(4);
        log.AddLog("[SETTINGS][SAVE] Save settings\n");
    }
    static void loadPacketsFromFile(const std::string& filename, std::vector<layoutPacket>& packets) {
        std::ifstream file(filename);
        if (file.is_open()) {
            json j;
            file >> j;
            packets = j.get<std::vector<layoutPacket>>();
            log.AddLog("[SETTINGS][LOAD] File loaded\n");
        }
        else {
            log.AddLog("[SETTINGS][LOAD] File not found\n");
        }
    }
    static void DragWindow(HWND hwnd) {
        POINT p;
        GetCursorPos(&p);  // Получаем текущую позицию мыши
        static POINT offset = { 0, 0 };
        static bool isDragging = false;

        RECT rect;
        GetClientRect(hwnd, &rect);
        MapWindowPoints(hwnd, nullptr, (LPPOINT)&rect, 2);

        POINT mousePos = { p.x, p.y };
        //if (mousePos.y >= rect.top && mousePos.y <= rect.top + 20) {
        //    log.AddLog("TEST MOVE WINDOW\n");

        //}
        // Проверяем, если нажата левая кнопка мыши
        if (ImGui::IsMouseClicked(0)) {
            

            // Проверяем, кликаем ли по верхней части окна (например, 20 пикселей сверху)
            if (mousePos.y >= rect.top && mousePos.y <= rect.top + 20) {
                isDragging = true;
                offset.x = p.x - rect.left;
                offset.y = p.y - rect.top;
            }
        }

        

        if (isDragging) {
            // Перемещаем окно, используя SetWindowPos
            
            SetWindowPos(hwnd, nullptr, p.x - offset.x, p.y - offset.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

            // Когда кнопка мыши отпущена, останавливаем перетаскивание
            if (ImGui::IsMouseReleased(0)) {
                isDragging = false;
            }
        }
    }
};