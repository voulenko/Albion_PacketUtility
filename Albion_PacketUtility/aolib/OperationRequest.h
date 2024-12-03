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

class APACKETLIB_API OperationRequest {
public:
    uint8_t OperationCode;
    std::vector<std::pair<uint8_t, DeserializedValue>> Parameters;

    OperationRequest(uint8_t operationCode, std::vector<std::pair<uint8_t, DeserializedValue>> parameters);
    ~OperationRequest();
};
