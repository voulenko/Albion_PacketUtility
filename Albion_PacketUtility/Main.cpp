#include "imGuiManager.h"
#include <iostream>

int main() {
    setlocale(LC_ALL, "Russian"); // Для консольного вывода если потребуется на русском
    ImGuiManager app;
    ImGuiRenderer::done = false;

    while (true) {
        if (ImGuiRenderer::done) break; // Фишка чтобы после закрытия окна завершался весь процесс программы, если закоментен то закрывается только Окно(кроме консоли) 
        ImGuiRenderer::doRender();
    }

    return 0;
}