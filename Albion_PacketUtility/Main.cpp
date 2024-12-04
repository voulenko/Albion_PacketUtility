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

struct tcphdr {
    unsigned short source;
    unsigned short dest;
    unsigned int seq;
    unsigned int ack_seq;
    unsigned char doff : 4, fin : 1, syn : 1, rst : 1, psh : 1, ack : 1, urg : 1;
    unsigned short window;
    unsigned short check;
    unsigned short urg_ptr;
};
struct SPacketHeader
{
    unsigned short mPacketSz;
    unsigned char mEncrypt;
    unsigned char mSeqNo;
    unsigned short mPacketId;
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

void analyzePacket(const unsigned char* packet, size_t packetSize) {
    struct iphdr* ipHeader = (struct iphdr*)packet;
    size_t ipHeaderSize = ipHeader->ihl * 4;

    struct tcphdr* tcpHeader = (struct tcphdr*)(packet + ipHeaderSize);
    size_t tcpHeaderSize = tcpHeader->doff * 4;

    // Размер полезной нагрузки
    size_t payloadSize = packetSize - (ipHeaderSize + tcpHeaderSize);
    if (payloadSize > 0) {
        const unsigned char* payload = packet + ipHeaderSize + tcpHeaderSize;
        std::vector<uint8_t> payloadVector(payload, payload + payloadSize);
            std::cout << "Payload: ";
            for (size_t i = 0; i < payloadVector.size(); ++i) {
                // Выводим байт в шестнадцатеричной системе с ведущими нулями
                std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)payloadVector[i];
                // Добавляем тире, если это не последний байт
                if (i < payloadVector.size() - 1) {
                    std::cout << "-";
                }
            }
            std::cout << std::endl;
            parser.ReceivePacket(payloadVector); // Получили пейлоад отправили сразу в парсер
        
    }
    else {
        std::cout << "Полезной нагрузки нет." << std::endl;
    }
}

void startCap() {
    const char* filter = "true";
    HANDLE handle = WinDivertOpen(filter, WINDIVERT_LAYER_NETWORK, 0, 0);
    if (handle == INVALID_HANDLE_VALUE) {
        std::cerr << "Не удалось открыть фильтр: " << GetLastError() << std::endl;
        return;
    }

    std::cout << "Перехват запущен..." << std::endl;

    UINT packetSize = 0;
    char packet[65535];
    WINDIVERT_ADDRESS addr;

    while (true) {
        if (WinDivertRecv(handle, packet, sizeof(packet), &packetSize, &addr)) {
            if (packetSize < sizeof(iphdr) + sizeof(tcphdr)) {
                std::cerr << "Пакет слишком мал для заголовков IP и TCP." << std::endl;
                continue;
            }

            struct iphdr* ipHeader = (struct iphdr*)packet;
            struct tcphdr* tcpHeader = (struct tcphdr*)(packet + sizeof(struct iphdr));

            char srcIP[INET_ADDRSTRLEN];
            char dstIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &ipHeader->ip_src, srcIP, sizeof(srcIP));
            inet_ntop(AF_INET, &ipHeader->ip_dst, dstIP, sizeof(dstIP));
            if (ntohs(tcpHeader->dest) == 5057) {
                std::cout << "Обрабатываем DEST пакет с IP: " << srcIP << " -> " << dstIP << std::endl;

                // Здесь вызываем функцию для разбивки и анализа пакета
                /*processReceivedPacket(packet + sizeof(struct iphdr) + sizeof(struct tcphdr),
                    packetSize - (sizeof(struct iphdr) + sizeof(struct tcphdr)));*/
               
                try {
                    analyzePacket((unsigned char*)packet, packetSize);
                }
                catch (const std::exception& ex) {
                    std::cerr << "Ошибка в парсере: " << ex.what() << std::endl;
                }
            }
            if (ntohs(tcpHeader->source) == 5056) {
                std::cout << "Обрабатываем SOURCE пакет с IP: " << srcIP << " -> " << dstIP << std::endl;
                
                // Здесь вызываем функцию для разбивки и анализа пакета
                /*processReceivedPacket(packet + sizeof(struct iphdr) + sizeof(struct tcphdr),
                    packetSize - (sizeof(struct iphdr) + sizeof(struct tcphdr)));*/
                try {
                    analyzePacket((unsigned char*)packet, packetSize);
                }
                catch (const std::exception& ex) {
                    std::cerr << "Ошибка в парсере: " << ex.what() << std::endl;
                }
                
            }


        }
        if (!WinDivertSend(handle, packet, packetSize, NULL, &addr)) {
            std::cerr << "Не удалось отправить пакет: " << GetLastError() << std::endl;
        }
    }

    WinDivertClose(handle);
}

int main() {
    setlocale(LC_ALL, "Russian"); // Для консольного вывода если потребуется на русском
    ImGuiManager app;
    ImGuiRenderer::done = false;
    //std::string str = "00-00-00-03-36-BB-25-94-25-01-9B-10-01-00-00-00-00-00-00-14-00-00-00-00-00-00-00-04-00-00-02-3A-06-00-01-00-00-00-00-52-00-00-00-38-F3-03-01-00-00-2A-00-08-00-73-00-11-41-44-43-53-45-41-53-4F-4E-5F-31-31-40-32-30-32-34-01-6C-08-DD-0B-23-7C-EA-CC-69-02-79-00-00-73-03-78-00-00-00-00-08-79-00-00-73-09-79-00-00-73-FF-69-00-00-00-02-FD-6B-01-69-07-00-00-00-00-00-00-43-00-00-00-38-00-00-00-01-F3-04-03-00-02-00-6C-00-00-00-00-00-00-B7-82-01-78-00-00-00-1E-03-72-88-E6-7C-23-0B-DD-08-8F-6F-6E-C1-61-D8-3D-22-21-00-00-B0-40-64-E9-29-C1-31-1B-1C-22";
    std::string str = "00-00-00-01-00-00-08-99-06-00-01-00-00-00-00-0f-00-00-00-01-f3-01-00";
    std::vector<uint8_t> byteArray = ConvertStringToByteArray(str);

    std::thread t(startCap);

    
    parser.addResponseHandler(new DebugResponseHandler());
    parser.addRequestHandler(new DebugRequestHandler());
    parser.addEventHandler(new DebugEventHandler());
    
    //parser.ReceivePacket(byteArray);

    while (true) {
        if (ImGuiRenderer::done) break; // Фишка чтобы после закрытия окна завершался весь процесс программы, если закоментен то закрывается только Окно(кроме консоли) 
        ImGuiRenderer::doRender();
    }
    //t.join();
    return 0;
}