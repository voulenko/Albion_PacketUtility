#include "imGuiManager.h"
#include <iostream>

int main() {
    setlocale(LC_ALL, "Russian"); // ��� ����������� ������ ���� ����������� �� �������
    ImGuiManager app;
    ImGuiRenderer::done = false;

    while (true) {
        if (ImGuiRenderer::done) break; // ����� ����� ����� �������� ���� ���������� ���� ������� ���������, ���� ���������� �� ����������� ������ ����(����� �������) 
        ImGuiRenderer::doRender();
    }

    return 0;
}