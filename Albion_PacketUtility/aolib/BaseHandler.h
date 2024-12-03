#pragma once
#define APACKETLIB_EXPORTS
#ifdef APACKETLIB_EXPORTS
#define APACKETLIB_API __declspec(dllexport)
#else
#define APACKETLIB_API __declspec(dllimport)
#endif

#include <vector>
#include <Deserializer.h>

class APACKETLIB_API BaseHandler {
private:
    int code;
public:
    BaseHandler(int code) : code(code) {} ;  // �����������
    virtual ~BaseHandler() {}; // ����������� ����������
    virtual void handle(const std::vector<std::pair<uint8_t, DeserializedValue>>& parameters) = 0;  // ����������� �����
    int getCode() {
        return code;
    };  // ����� ��� ��������� ����
};

class APACKETLIB_API DebugHandler : public BaseHandler {
public:
    DebugHandler() : BaseHandler(0) {}
    virtual void handle(const std::vector<std::pair<uint8_t, DeserializedValue>>& parameters) {}; // �������������� �� baseHandler
    virtual void handle(int code, const std::vector<std::pair<uint8_t, DeserializedValue>>& parameters) = 0;  // ����������� �������� �����
};