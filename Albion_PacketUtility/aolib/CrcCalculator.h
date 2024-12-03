#pragma once

#ifdef APACKETLIB_EXPORTS  // ���� ������ ����� ��������� � ������� DLL
#define APACKETLIB_API __declspec(dllexport)
#else
#define APACKETLIB_API __declspec(dllimport)
#endif

#include <cstdint>
#include <vector>

class APACKETLIB_API CrcCalculator {
public:
    static uint32_t Calculate(const uint8_t* bytes, size_t size);
};
