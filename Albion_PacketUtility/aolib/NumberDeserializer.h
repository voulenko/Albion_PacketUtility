#pragma once

#ifdef APACKETLIB_EXPORTS  // Этот макрос будет определен в проекте DLL
#define APACKETLIB_API __declspec(dllexport)
#else
#define APACKETLIB_API __declspec(dllimport)
#endif
#include <cstdint>
#include <vector>

class APACKETLIB_API NumberDeserializer {
public:
    static void Deserialize(int32_t& value, const std::vector<uint8_t>& source, size_t& offset);
    static void Deserialize(int16_t& value, const std::vector<uint8_t>& source, size_t& offset);
};
