#include <winsock2.h>
#include <ws2tcpip.h>
#include "imGuiManager.h"
#include <iostream>
#include "windivert.h"
#include <thread>
#include "Parser.h"
#include "DebugResponseHandler.cpp"
#include "DebugRequestHandler.cpp"
#include "DebugEventHandler.cpp"
#include <iomanip>

#pragma comment(lib,"Ws2_32.lib")

Parser parser;

struct iphdr {
    unsigned char ihl : 4, version : 4;
    unsigned char tos;
    unsigned short tot_len;
    unsigned short id;
    unsigned short frag_off;
    unsigned char ttl;
    unsigned char protocol;
    unsigned short check;
    struct in_addr ip_src, ip_dst;
};

struct udphdr {
    unsigned short source; // Порт источника
    unsigned short dest;   // Порт назначения
    unsigned short len;    // Длина заголовка и данных
    unsigned short check;   // Контрольная сумма
};

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

void calculate_checksum(PVOID packet, UINT packetSize) {
    //std::cout << "CALCULATE" << std::endl;
    if (!WinDivertHelperCalcChecksums(packet, packetSize, 0, 0)) {
        std::cerr << "Failed to recalculate checksum." << std::endl;
    }
}


// Функция для преобразования unsigned short в big-endian
unsigned short toBigEndianShort(unsigned short value) {
    return (value >> 8) | (value << 8);
}

// Функция для преобразования unsigned int в big-endian
unsigned int toBigEndianInt(unsigned int value) {
    return ((value >> 24) & 0xFF) | ((value >> 8) & 0xFF00) | ((value << 8) & 0xFF0000) | ((value << 24) & 0xFF000000);
}


void toBigEndianTcphdr(struct udphdr* hdr) {
    hdr->source = toBigEndianShort(hdr->source);
    hdr->dest = toBigEndianShort(hdr->dest);
    hdr->len = toBigEndianShort(hdr->len);
    hdr->check = toBigEndianShort(hdr->check);
}

void startCap() {
    const char* filter = "udp.DstPort == 5056 or udp.SrcPort == 5056";
    HANDLE handle = WinDivertOpen(filter, WINDIVERT_LAYER_NETWORK, 0, 0);
    if (handle == INVALID_HANDLE_VALUE) {
        std::cerr << "Не удалось открыть фильтр: " << GetLastError() << std::endl;
        return;
    }

    std::cout << "Перехват запущен..." << std::endl;

    UINT packetSize = 0;
    unsigned char packet[0xFFFF];
    WINDIVERT_ADDRESS addr;

    

    while (true) {
        if (ImGuiRenderer::done) return;
        if(WinDivertRecv(handle, packet, sizeof(packet), &packetSize, &addr) && ImGuiRenderer::capture) {
            //std::cout << "TEST TEST TEST" << std::endl;
            if (packetSize < sizeof(iphdr) + sizeof(udphdr)) {
                std::cerr << "Пакет слишком мал для заголовков IP и TCP." << std::endl;
                continue;
            }
            // Создаём копию пакета для пересчёта контрольной суммы и корректировки заголовка TCP
            std::vector<uint8_t> nPacket(packet, packet + packetSize);

            // Получаем указатель на IP-заголовок
            struct iphdr* ipHeader = reinterpret_cast<struct iphdr*>(nPacket.data());

            // Пересчитываем контрольную сумму
            calculate_checksum(nPacket.data(), nPacket.size());

            // Получаем указатель на UDP-заголовок
            struct udphdr* udpHeader = reinterpret_cast<struct udphdr*>(nPacket.data() + sizeof(struct iphdr));

            // Конвертируем заголовок TCP в big-endian
            toBigEndianTcphdr(udpHeader);

            // Дублируем часть пакета, а именно полезную нагрузку (payload)
            size_t payloadSize = udpHeader->len - sizeof(struct udphdr); // Размер полезной нагрузки
            std::vector<uint8_t> payload(nPacket.begin() + sizeof(struct iphdr) + sizeof(struct udphdr),
                nPacket.begin() + sizeof(struct iphdr) + sizeof(struct udphdr) + payloadSize);

            char srcIP[INET_ADDRSTRLEN];
            char dstIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &ipHeader->ip_src, srcIP, sizeof(srcIP));
            inet_ntop(AF_INET, &ipHeader->ip_dst, dstIP, sizeof(dstIP));

            auto now = std::chrono::system_clock::now();
            std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

            // Преобразуем в строку
            char timeBuffer[100];
            std::tm timeInfo;
            localtime_s(&timeInfo, &currentTime);  // safe localtime_s instead of unsafe localtime
            std::strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", &timeInfo);

            if(udpHeader->dest == 5056) {
                ImGuiRenderer::log.AddLog("[%s][PACKET] Client->Server payload[%d]\n", timeBuffer, payload.size());
                parser.ReceivePacket(payload);
            }
            if(udpHeader->source == 5056) {
                ImGuiRenderer::log.AddLog("[%s][PACKET] Server->Client payload[%d]\n", timeBuffer, payload.size());
                parser.ReceivePacket(payload);
            }
            //if (ntohs(udpHeader->dest) == 5056) {
            //    //std::cout << "Обрабатываем DEST пакет с IP: " << srcIP << " -> " << dstIP << ": " << packetSize << std::endl;
            //    analyzePacket((unsigned char*)packet, packetSize);
            //}
            //if (ntohs(udpHeader->source) == 5056) {
            //    //std::cout << "Обрабатываем SOURCE пакет с IP: " << srcIP << " -> " << dstIP << ": " << packetSize << std::endl;
            //    analyzePacket((unsigned char*)packet, packetSize);
            //}

            

        }

        if (!WinDivertSend(handle, packet, packetSize, NULL, &addr)) {
            std::cerr << "Не удалось отправить пакет: " << GetLastError() << std::endl;
        }
        
    }

    WinDivertClose(handle);
}



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    setlocale(LC_ALL, "Russian"); // Для консольного вывода если потребуется на русском

    ImGuiManager::Overlay = false;

    ImGuiManager app;
    ImGuiRenderer::done = false;
    ImGuiRenderer::capture = true;

    std::thread t(startCap);

    
    parser.addResponseHandler(new DebugResponseHandler());
    parser.addRequestHandler(new DebugRequestHandler());
    parser.addEventHandler(new DebugEventHandler());

    // Load Settings Packet
    

    for (int i = 0; i < 500; i++) ImGuiRenderer::packetsSettings.push_back({ i, "", "", true, MessageType::Event });
    for (int i = 0; i < 500; i++) ImGuiRenderer::packetsSettings.push_back({ i, "", "", true, MessageType::OperationResponse });
    for (int i = 0; i < 500; i++) ImGuiRenderer::packetsSettings.push_back({ i, "", "", true, MessageType::OperationRequest });

    while (true) {
        if (ImGuiRenderer::done) break; // Фишка чтобы после закрытия окна завершался весь процесс программы, если закоментен то закрывается только Окно(кроме консоли) 
        ImGuiRenderer::doRender();
    }
    t.join();
    return 0;
}