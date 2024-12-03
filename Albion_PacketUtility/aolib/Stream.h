#pragma once

#ifdef APACKETLIB_EXPORTS
#define APACKETLIB_API __declspec(dllexport)
#else
#define APACKETLIB_API __declspec(dllimport)
#endif

#include <vector>
#include <cstddef> // Для size_t

class APACKETLIB_API Stream {
private:
    
    size_t position = 0;         // Текущая позиция
    size_t length = 0;           // Длина данных в буфере

    void ExpandIfNeeded(size_t requiredSize); // Расширяет буфер, если требуется

public:
    Stream(size_t size = 0); // Конструктор с размером
    Stream(const std::vector<uint8_t>& initialBuffer); // Конструктор с начальным буфером
    std::vector<uint8_t> buffer; // Внутренний буфер
    ~Stream();

    size_t Capacity() const;         // Возвращает ёмкость буфера
    size_t Length() const;           // Возвращает текущую длину данных
    size_t Position() const;         // Возвращает текущую позицию
    void SetPosition(size_t newPosition); // Устанавливает новую позицию
    void SetLength(size_t newLength);     // Устанавливает длину данных

    void Flush();                       // Заглушка для совместимости
    size_t Read(std::vector<uint8_t>& outBuffer, size_t offset, size_t count); // Чтение данных
    int ReadByte();                     // Чтение одного байта
    void Write(const std::vector<uint8_t>& inputBuffer, size_t offset, size_t count); // Запись данных
    long Seek(long offset, int origin); // Перемещение по буферу
};