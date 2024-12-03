#pragma once

#ifdef APACKETLIB_EXPORTS  // Ётот макрос будет определен в проекте DLL
#define APACKETLIB_API __declspec(dllexport)
#else
#define APACKETLIB_API __declspec(dllimport)
#endif

#include <iostream>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <cstring>
#include "NumberDeserializer.h"
#include <NumberSerializer.h>
#include <CrcCalculator.h>
#include "OperationRequest.h"
#include "OperationResponse.h"
#include "EventData.h"
#include "Deserializer.h"
#include <BaseHandler.h>

constexpr int CommandHeaderLength = 12;
constexpr int PhotonHeaderLength = 12;

enum class CommandType : uint8_t {
    Disconnect = 4,
    SendReliable = 6,
    SendUnreliable = 7,
    SendFragment = 8
};

enum class MessageType : uint8_t {
    OperationRequest = 2,
    OperationResponse = 3,
    Event = 4
};

struct SegmentedPackage {
    int totalLength;
    int bytesWritten;
    std::vector<uint8_t> totalPayload;

    SegmentedPackage(int length) : totalLength(length), bytesWritten(0), totalPayload(length) {}
};

class Parser {
public:
    void ReceivePacket(const std::vector<uint8_t>& payload)
    {
        //std::cout << "TEST: ReceivePacket" << std::endl;
        const int PhotonHeaderLength = 12;

        if (payload.size() < PhotonHeaderLength)
        {
            return;
        }

        size_t offset = 0;

        int16_t peerId = 0;
        NumberDeserializer::Deserialize(peerId, payload, offset);

        uint8_t flags = 0;
        flags = readByte(payload, offset);

        uint8_t commandCount = 0;
        commandCount = readByte(payload, offset);

        int timestamp = 0;
        NumberDeserializer::Deserialize(timestamp, payload, offset);

        int challenge = 0;
        NumberDeserializer::Deserialize(challenge, payload, offset);

        bool isEncrypted = (flags == 1);
        bool isCrcEnabled = (flags == 0xCC);

        if (isEncrypted)
        {
            return;
        }

        // ѕроверка CRC
        if (isCrcEnabled)
        {
            size_t ignoredOffset = 0;
            int crc = 0;
            NumberDeserializer::Deserialize(crc, payload, ignoredOffset);

            std::vector<uint8_t> writablePayload = payload;
            NumberSerializer::Serialize(0, writablePayload, offset);

            if (crc != CrcCalculator::Calculate(payload.data(), payload.size()))
            {
                return;
            }
        }
        for (int commandIdx = 0; commandIdx < commandCount; commandIdx++)
        {
            handleCommand(payload, offset);
        }
    }

    void addResponseHandler(BaseHandler* handler) {
        int code = handler->getCode();
        if (responseHandlers.find(code) != responseHandlers.end()) {
            std::cerr << "[addResponseHandler] event code " << code << " already exists" << std::endl;
            delete handler;
            return;
        }
        responseHandlers[code] = std::unique_ptr<BaseHandler>(handler);
    }

    void addRequestHandler(BaseHandler* handler) {
        int code = handler->getCode();
        if (requestHandlers.find(code) != requestHandlers.end()) {
            std::cerr << "[addRequestHandler] event code " << code << " already exists" << std::endl;
            delete handler;
            return;
        }
        requestHandlers[code] = std::unique_ptr<BaseHandler>(handler);
    }

    void addEventHandler(BaseHandler* handler) {
        int code = handler->getCode();
        if (eventHandlers.find(code) != eventHandlers.end()) {
            std::cerr << "[addEventHandler] event code " << code << " already exists" << std::endl;
            delete handler;
            return;
        }
        eventHandlers[code] = std::unique_ptr<BaseHandler>(handler);
    }

    void handleResponse(int code, const std::vector<std::pair<uint8_t, DeserializedValue>>& parameters) {
        auto globalHandler = responseHandlers.find(0);
        if (globalHandler != responseHandlers.end()) {
            static_cast<DebugHandler*>(globalHandler->second.get())->handle(code, parameters);
        }
        auto it = responseHandlers.find(code);
        if (it != responseHandlers.end()) it->second->handle(parameters);
        //else std::cerr << "[handleResponse] not handler code " << code << "!" << std::endl;
    }
    void handleRequest(int code, const std::vector<std::pair<uint8_t, DeserializedValue>>& parameters) {
        auto globalHandler = requestHandlers.find(0);
        if (globalHandler != requestHandlers.end()) {
            static_cast<DebugHandler*>(globalHandler->second.get())->handle(code, parameters);
        }
        auto it = requestHandlers.find(code);
        if (it != requestHandlers.end()) it->second->handle(parameters);
        //else std::cerr << "[handleRequest] not handler code " << code << "!" << std::endl;
    }

    void handleEvent(int code, const std::vector<std::pair<uint8_t, DeserializedValue>>& parameters) {
        auto globalHandler = eventHandlers.find(0);
        if (globalHandler != eventHandlers.end()) {
            static_cast<DebugHandler*>(globalHandler->second.get())->handle(code, parameters);
        }
        auto it = eventHandlers.find(code);
        if (it != eventHandlers.end()) it->second->handle(parameters);
        //else std::cerr << "[handleEvent] not handler code " << code << "!" << std::endl;
    }

protected:
    void onRequest(uint8_t operationCode, const std::vector<std::pair<uint8_t, DeserializedValue>>& parameters) {
        //std::cout << "TEST REQ " << std::to_string(operationCode) << std::endl;
        handleResponse(static_cast<int>(operationCode), parameters);
    }
    void onResponse(uint8_t operationCode, int16_t returnCode, const std::string& debugMessage, const std::vector<std::pair<uint8_t, DeserializedValue>>& parameters) {
        //std::cout << "TEST RES " << std::to_string(operationCode) << std::endl;
        handleRequest(static_cast<int>(operationCode), parameters);
    }
    void onEvent(uint8_t code, const std::vector<std::pair<uint8_t, DeserializedValue>>& parameters) {
        //std::cout << "TEST EVE " << std::to_string(code) << std::endl;
        handleEvent(static_cast<int>(code), parameters);
    }

private:
    std::unordered_map<int, SegmentedPackage> pendingSegments;

    std::unordered_map<int, std::unique_ptr<BaseHandler>> responseHandlers;
    std::unordered_map<int, std::unique_ptr<BaseHandler>> requestHandlers;
    std::unordered_map<int, std::unique_ptr<BaseHandler>> eventHandlers;

    void handleCommand(const std::vector<uint8_t>& source, size_t& offset) {
        uint8_t commandType = readByte(source, offset);
        uint8_t channelId = readByte(source, offset);
        uint8_t commandFlags = readByte(source, offset);
        offset++; // Skip 1 byte

        int deCommandLength = 0;
        NumberDeserializer::Deserialize(deCommandLength, source, offset);

        int sequernceNumber = 0;
        NumberDeserializer::Deserialize(sequernceNumber, source, offset);
        deCommandLength -= CommandHeaderLength;
        int commandLength = deCommandLength;




        switch (static_cast<CommandType>(commandType)) {
        case CommandType::Disconnect:
            return;
        case CommandType::SendUnreliable:
            offset += 4;
            commandLength -= 4;
            [[fallthrough]];
        case CommandType::SendReliable:
            handleSendReliable(source, offset, commandLength);
            break;
        case CommandType::SendFragment:
            handleSendFragment(source, offset, commandLength);
            break;
        default:
            offset += commandLength;
            break;
        }
    }

    void handleSendReliable(const std::vector<uint8_t>& source, size_t& offset, int& commandLength) {
        offset++; // Skip 1 byte
        commandLength--;

        uint8_t messageType = readByte(source, offset);
        commandLength--;

        int operationLength = commandLength;
        Stream payload = Stream(operationLength);
        payload.Write(source, offset, operationLength);
        payload.Seek(0L, 0);
        offset += operationLength;

        switch (static_cast<MessageType>(messageType)) {
        case MessageType::OperationRequest: {
            OperationRequest requestData = Deserializer::DeserializeOperationRequest(payload);
            onRequest(requestData.OperationCode, requestData.Parameters);
        }
                                          break;
        case MessageType::OperationResponse: {
            OperationResponse responseData = Deserializer::DeserializeOperationResponse(payload);
            onResponse(responseData.OperationCode, responseData.ReturnCode, responseData.DebugMessage, responseData.Parameters);
        }
                                           break;
        case MessageType::Event: {
            EventData eventData = Deserializer::DeserializeEventData(payload);
            onEvent(eventData.Code, eventData.Parameters);
        }
                               break;
        }
    }

    void handleSendFragment(const std::vector<uint8_t>& source, size_t& offset, int& commandLength) {
        int startSequenceNumber = 0;
        NumberDeserializer::Deserialize(startSequenceNumber, source, offset);
        commandLength -= 4;
        int fragmentCount = 0;
        NumberDeserializer::Deserialize(fragmentCount, source, offset);
        commandLength -= 4;
        int fragmentNumber = 0;
        NumberDeserializer::Deserialize(fragmentNumber, source, offset);
        commandLength -= 4;
        int totalLength = 0;
        NumberDeserializer::Deserialize(totalLength, source, offset);
        commandLength -= 4;
        int fragmentOffset = 0;
        NumberDeserializer::Deserialize(startSequenceNumber, source, offset);
        commandLength -= 4;

        handleSegmentedPayload(startSequenceNumber, totalLength, commandLength, fragmentOffset, source, offset);
    }

    void handleSegmentedPayload(int startSequenceNumber, int totalLength, int fragmentLength, int fragmentOffset,
        const std::vector<uint8_t>& source, size_t& offset) {
        auto& segmentedPackage = getSegmentedPackage(startSequenceNumber, totalLength);

        std::copy(source.begin() + offset, source.begin() + offset + fragmentLength,
            segmentedPackage.totalPayload.begin() + fragmentOffset);

        offset += fragmentLength;
        segmentedPackage.bytesWritten += fragmentLength;

        if (segmentedPackage.bytesWritten >= segmentedPackage.totalLength) {
            pendingSegments.erase(startSequenceNumber);
            handleFinishedSegmentedPackage(segmentedPackage.totalPayload);
        }
    }

    SegmentedPackage& getSegmentedPackage(int startSequenceNumber, int totalLength) {
        auto it = pendingSegments.find(startSequenceNumber);
        if (it != pendingSegments.end()) {
            return it->second;
        }

        return pendingSegments.emplace(startSequenceNumber, SegmentedPackage(totalLength)).first->second;
    }

    void handleFinishedSegmentedPackage(const std::vector<uint8_t>& totalPayload) {
        size_t offset = 0;
        int commandLength = static_cast<int>(totalPayload.size());
        handleSendReliable(totalPayload, offset, commandLength);
    }

    uint8_t readByte(const std::vector<uint8_t>& source, size_t& offset) {
        return source[offset++];
    }
};
