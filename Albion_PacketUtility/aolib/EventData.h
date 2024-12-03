#pragma once

#ifdef APACKETLIB_EXPORTS  // Этот макрос будет определен в проекте DLL
#define APACKETLIB_API __declspec(dllexport)
#else
#define APACKETLIB_API __declspec(dllimport)
#endif

#include <cstdint>
#include <unordered_map>
#include <string>
#include <memory>
#include <typeindex>
#include <Deserializer.h>

class APACKETLIB_API EventData {
public:
    uint8_t Code;
    std::vector<std::pair<uint8_t, DeserializedValue>> Parameters;

    EventData(uint8_t code, std::vector<std::pair<uint8_t, DeserializedValue>> params);
    ~EventData();
};