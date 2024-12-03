#include "imGuiManager.h"
#include <iostream>
#include "Parser.h"
#include "DebugResponseHandler.cpp"
#include "DebugRequestHandler.cpp"
#include "DebugEventHandler.cpp"

std::vector<uint8_t> ConvertStringToByteArray(const std::string& str) {
    std::vector<uint8_t> byteArray;

    // Перебираем строку и парсим каждую пару символов
    for (size_t i = 0; i < str.length(); i += 3) {  // Шаг 3, потому что между байтами стоит дефис
        std::string byteStr = str.substr(i, 2);  // Берем два символа (байт)

        // Преобразуем строку в число в шестнадцатеричной системе
        uint8_t byte = static_cast<uint8_t>(std::stoi(byteStr, nullptr, 16));
        byteArray.push_back(byte);
    }

    return byteArray;
}

int main() {
    setlocale(LC_ALL, "Russian"); // Для консольного вывода если потребуется на русском
    ImGuiManager app;
    ImGuiRenderer::done = false;
    std::string str = "00-00-00-03-36-BB-25-94-25-01-9B-10-01-00-00-00-00-00-00-14-00-00-00-00-00-00-00-04-00-00-02-3A-06-00-01-00-00-00-00-52-00-00-00-38-F3-03-01-00-00-2A-00-08-00-73-00-11-41-44-43-53-45-41-53-4F-4E-5F-31-31-40-32-30-32-34-01-6C-08-DD-0B-23-7C-EA-CC-69-02-79-00-00-73-03-78-00-00-00-00-08-79-00-00-73-09-79-00-00-73-FF-69-00-00-00-02-FD-6B-01-69-07-00-00-00-00-00-00-43-00-00-00-38-00-00-00-01-F3-04-03-00-02-00-6C-00-00-00-00-00-00-B7-82-01-78-00-00-00-1E-03-72-88-E6-7C-23-0B-DD-08-8F-6F-6E-C1-61-D8-3D-22-21-00-00-B0-40-64-E9-29-C1-31-1B-1C-22";
    std::vector<uint8_t> byteArray = ConvertStringToByteArray(str);

    Parser parser;
    parser.addResponseHandler(new DebugResponseHandler());
    parser.addRequestHandler(new DebugRequestHandler());
    parser.addEventHandler(new DebugEventHandler());
    
    parser.ReceivePacket(byteArray);

    while (true) {
        if (ImGuiRenderer::done) break; // Фишка чтобы после закрытия окна завершался весь процесс программы, если закоментен то закрывается только Окно(кроме консоли) 
        ImGuiRenderer::doRender();
    }

    return 0;
}