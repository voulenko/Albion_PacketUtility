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
#include <Deserializer.h>

class APACKETLIB_API OperationResponse {
public:
    uint8_t OperationCode;
    int16_t ReturnCode;
    std::string DebugMessage;
    std::vector<std::pair<uint8_t, DeserializedValue>> Parameters;

    OperationResponse(uint8_t operationCode, int16_t returnCode, const std::string debugMessage, std::vector<std::pair<uint8_t, DeserializedValue>> parameters);
    ~OperationResponse();
};
