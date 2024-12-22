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
        if(WinDivertRecv(handle, packet, sizeof(packet), &packetSize, &addr)) {
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
            if(udpHeader->dest == 5056) {
                parser.ReceivePacket(payload);
            }
            if(udpHeader->source == 5056) {
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

int main() {
    setlocale(LC_ALL, "Russian"); // Для консольного вывода если потребуется на русском
    ImGuiManager app;
    ImGuiRenderer::done = false;
    //std::string str = "00-00-00-03-36-BB-25-94-25-01-9B-10-01-00-00-00-00-00-00-14-00-00-00-00-00-00-00-04-00-00-02-3A-06-00-01-00-00-00-00-52-00-00-00-38-F3-03-01-00-00-2A-00-08-00-73-00-11-41-44-43-53-45-41-53-4F-4E-5F-31-31-40-32-30-32-34-01-6C-08-DD-0B-23-7C-EA-CC-69-02-79-00-00-73-03-78-00-00-00-00-08-79-00-00-73-09-79-00-00-73-FF-69-00-00-00-02-FD-6B-01-69-07-00-00-00-00-00-00-43-00-00-00-38-00-00-00-01-F3-04-03-00-02-00-6C-00-00-00-00-00-00-B7-82-01-78-00-00-00-1E-03-72-88-E6-7C-23-0B-DD-08-8F-6F-6E-C1-61-D8-3D-22-21-00-00-B0-40-64-E9-29-C1-31-1B-1C-22";
    std::string str = "00-00-00-0d-23-a3-18-55-1f-97-c2-8f-06-00-01-00-00-00-00-92-00-00-00-23-f3-04-01-00-02-00-79-00-78-6f-01-01-01-01-01-01-01-01-01-01-01-01-01-01-01-00-01-00-01-01-01-01-01-00-01-01-01-01-01-01-01-01-01-01-01-01-00-01-01-01-01-00-01-01-01-01-00-01-01-01-01-01-01-01-01-01-00-01-01-01-01-01-01-01-01-01-01-01-01-01-00-01-01-01-01-01-01-00-01-01-01-01-01-01-00-01-01-01-00-01-01-01-01-01-01-01-01-01-01-01-01-00-01-01-00-01-01-01-01-01-01-01-01-01-01-01-01-01-00-01-fc-6b-01-ed-06-00-01-00-00-00-00-63-00-00-00-24-f3-04-01-00-05-00-6b-01-83-01-73-00-27-53-57-41-4d-50-5f-52-45-44-5f-46-41-43-54-49-4f-4e-5f-57-41-52-46-41-52-45-5f-4f-55-54-50-4f-53-54-5f-54-4f-57-45-52-02-73-00-0e-4f-55-54-50-4f-53-54-5f-53-41-46-45-5f-35-03-79-00-02-66-c3-1c-00-00-43-58-00-00-fc-6b-01-64-06-00-01-00-00-00-00-41-00-00-00-25-f3-04-01-00-09-00-6b-01-83-01-62-05-02-62-05-03-69-00-00-00-01-04-69-00-00-00-00-05-69-00-98-96-80-06-69-00-98-96-80-07-6c-08-dd-20-e4-af-47-5a-80-fc-6b-01-65-06-00-01-00-00-00-00-63-00-00-00-26-f3-04-01-00-05-00-6b-08-61-01-73-00-27-53-57-41-4d-50-5f-52-45-44-5f-46-41-43-54-49-4f-4e-5f-57-41-52-46-41-52-45-5f-4f-55-54-50-4f-53-54-5f-54-4f-57-45-52-02-73-00-0e-4f-55-54-50-4f-53-54-5f-53-41-46-45-5f-35-03-79-00-02-66-c3-a3-00-00-c3-98-00-00-fc-6b-01-64-06-00-01-00-00-00-00-41-00-00-00-27-f3-04-01-00-09-00-6b-08-61-01-62-05-02-62-05-03-69-00-00-00-01-04-69-00-00-00-00-05-69-00-98-96-80-06-69-00-98-96-80-07-6c-08-dd-20-e4-43-01-83-56-fc-6b-01-65-06-00-01-00-00-00-00-63-00-00-00-28-f3-04-01-00-05-00-6b-08-64-01-73-00-27-53-57-41-4d-50-5f-52-45-44-5f-46-41-43-54-49-4f-4e-5f-57-41-52-46-41-52-45-5f-4f-55-54-50-4f-53-54-5f-54-4f-57-45-52-02-73-00-0e-4f-55-54-50-4f-53-54-5f-53-41-46-45-5f-35-03-79-00-02-66-c3-b2-00-00-43-ad-00-00-fc-6b-01-64-06-00-01-00-00-00-00-41-00-00-00-29-f3-04-01-00-09-00-6b-08-64-01-62-05-02-62-05-03-69-00-00-00-01-04-69-00-00-00-00-05-69-00-98-96-80-06-69-00-98-96-80-07-6c-08-dd-20-de-b3-b4-29-73-fc-6b-01-65-06-00-01-00-00-00-00-63-00-00-00-2a-f3-04-01-00-05-00-6b-08-67-01-73-00-27-53-57-41-4d-50-5f-52-45-44-5f-46-41-43-54-49-4f-4e-5f-57-41-52-46-41-52-45-5f-4f-55-54-50-4f-53-54-5f-54-4f-57-45-52-02-73-00-0e-4f-55-54-50-4f-53-54-5f-53-41-46-45-5f-35-03-79-00-02-66-43-74-00-00-c3-84-00-00-fc-6b-01-64-06-00-01-00-00-00-00-41-00-00-00-2b-f3-04-01-00-09-00-6b-08-67-01-62-05-02-62-05-03-69-00-00-00-01-04-69-00-00-00-00-05-69-00-98-96-80-06-69-00-98-96-80-07-6c-08-dd-20-de-b3-b4-50-92-fc-6b-01-65-06-00-01-00-00-00-00-63-00-00-00-2c-f3-04-01-00-05-00-6b-08-6a-01-73-00-27-53-57-41-4d-50-5f-52-45-44-5f-46-41-43-54-49-4f-4e-5f-57-41-52-46-41-52-45-5f-4f-55-54-50-4f-53-54-5f-54-4f-57-45-52-02-73-00-0e-4f-55-54-50-4f-53-54-5f-53-41-46-45-5f-35-03-79-00-02-66-c3-58-00-00-c1-c0-00-00-fc-6b-01-64-06-00-01-00-00-00-00-41-00-00-00-2d-f3-04-01-00-09-00-6b-08-6a-01-62-05-02-62-05-03-69-00-00-00-01-04-69-00-00-00-00-05-69-00-98-96-80-06-69-00-98-96-80-07-6c-08-dd-20-e3-f1-14-48-85-fc-6b-01-65-06-00-01-00-00-00-00-7e-00-00-00-2e-f3-84-cc-94-97-62-e0-07-d3-f4-8f-8c-f8-f5-ab-93-75-fe-db-7a-6b-07-46-ef-ad-62-8a-3d-67-53-07-7a-60-45-6d-63-fa-7a-74-b2-a8-30-5b-f3-a8-26-bd-a2-dc-5c-56-77-b1-ba-ec-cd-b0-1c-91-0f-6d-0c-d8-ed-0d-aa-5a-ec-b2-26-c9-64-c8-1c-04-a9-be-1b-61-3c-70-21-ea-9c-da-dd-5f-09-42-14-5e-40-4e-9d-52-f6-a1-1d-0d-c1-ca-c1-b4-74-8e-0f-e3-b0-70-b9-be-90-e3-a8-06-00-01-00-00-00-00-19-00-00-00-2f-f3-04-01-00-02-00-6b-ff-ff-fc-6b-01-0d";
    //std::string str = "00-00-00-01-00-00-08-99-06-00-01-00-00-00-00-0f-00-00-00-01-f3-01-00";
    //std::string str = "01-00-02-00-79-00-78-6f-01-01-01-01-01-01-01-01-01-01-01-01-01-01-01-00-01-00-01-01-01-01-01-00-01-01-01-01-01-01-01-01-01-01-01-01-00-01-01-01-01-00-01-01-01-01-00-01-01-01-01-01-01-01-01-01-00-01-01-01-01-01-01-01-01-01-01-01-01-01-00-01-01-01-01-01-01-00-01-01-01-01-01-01-00-01-01-01-00-01-01-01-01-01-01-01-01-01-01-01-01-00-01-01-00-01-01-01-01-01-01-01-01-01-01-01-01-01-00-01-fc-6b-01-ed";

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