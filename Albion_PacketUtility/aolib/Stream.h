#pragma once

#ifdef APACKETLIB_EXPORTS
#define APACKETLIB_API __declspec(dllexport)
#else
#define APACKETLIB_API __declspec(dllimport)
#endif

#include <vector>
#include <cstddef> // ��� size_t

class APACKETLIB_API Stream {
private:
    
    size_t position = 0;         // ������� �������
    size_t length = 0;           // ����� ������ � ������

    void ExpandIfNeeded(size_t requiredSize); // ��������� �����, ���� ���������

public:
    Stream(size_t size = 0); // ����������� � ��������
    Stream(const std::vector<uint8_t>& initialBuffer); // ����������� � ��������� �������
    std::vector<uint8_t> buffer; // ���������� �����
    ~Stream();

    size_t Capacity() const;         // ���������� ������� ������
    size_t Length() const;           // ���������� ������� ����� ������
    size_t Position() const;         // ���������� ������� �������
    void SetPosition(size_t newPosition); // ������������� ����� �������
    void SetLength(size_t newLength);     // ������������� ����� ������

    void Flush();                       // �������� ��� �������������
    size_t Read(std::vector<uint8_t>& outBuffer, size_t offset, size_t count); // ������ ������
    int ReadByte();                     // ������ ������ �����
    void Write(const std::vector<uint8_t>& inputBuffer, size_t offset, size_t count); // ������ ������
    long Seek(long offset, int origin); // ����������� �� ������
};