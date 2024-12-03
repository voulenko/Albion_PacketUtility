#pragma once

#ifdef APACKETLIB_EXPORTS  // Этот макрос будет определен в проекте DLL
#define APACKETLIB_API __declspec(dllexport)
#else
#define APACKETLIB_API __declspec(dllimport)
#endif

#include "Stream.h"

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include <memory>
#include <typeinfo>
#include <cstring> // для std::memcpy
#include <typeindex>
#include <unordered_map>

enum class Type {
    Unknown = 0,            // \0
    Null = 42,              // *
    Dictionary = 68,        // D
    StringArray = 97,       // a
    Byte = 98,              // b
    Double = 100,           // d
    EventData = 101,        // e
    Float = 102,            // f
    Integer = 105,          // i
    Hashtable = 104,        // j
    Short = 107,            // k
    Long = 108,             // l
    IntegerArray = 110,     // n
    Boolean = 111,          // o
    OperationResponse = 112,// p
    OperationRequest = 113, // q
    String = 115,           // s
    ByteArray = 120,        // x
    Array = 121,            // y
    ObjectArray = 122,      // z
};

struct DeserializedValue {
    std::shared_ptr<void> value;
    Type type;

    // Конструктор по умолчанию
    DeserializedValue() : value(nullptr), type(Type::Unknown) {}

    // Конструктор для конкретных типов
    template <typename T>
    DeserializedValue(std::shared_ptr<T> v, Type t)
        : value(std::move(v)), type(t) {}

    // Конструктор для nullptr_t
    DeserializedValue(std::nullptr_t)
        : value(nullptr), type(Type::Unknown) {}

    std::string getValueStr() const {
        //if (!value) {
        //    std::cout << "Value is null" << std::endl;
        //}
        // fixed a2
        switch (type) 
        {
            case Type::Long:
                return std::to_string(*std::static_pointer_cast<int64_t>(value));
            case Type::Integer:
                return std::to_string(*std::static_pointer_cast<int32_t>(value));
            case Type::String:
                return *std::static_pointer_cast<std::string>(value);
            case Type::Boolean:
                return *std::static_pointer_cast<bool>(value) ? "true" : "false";
            case Type::Byte:
                return std::to_string(*std::static_pointer_cast<uint8_t>(value));
            case Type::Double:
                return std::to_string(*std::static_pointer_cast<double>(value));
            case Type::Float:
                return std::to_string(*std::static_pointer_cast<float>(value));
            case Type::Short:
                return std::to_string(*std::static_pointer_cast<int16_t>(value));
            case Type::ByteArray: {
                const auto& byteArray = *std::static_pointer_cast<std::vector<uint8_t>>(value);
                std::string hexString;
                //for (size_t i = 0; i < byteArray.size(); ++i) {
                //    if (i > 0) {
                //        hexString += '-'; // Добавляем '-' перед каждым байтом, кроме первого
                //    }
                //    hexString += "0123456789ABCDEF"[byteArray[i] >> 4];
                //    hexString += "0123456789ABCDEF"[byteArray[i] & 0x0F];
                //}
                // Добавляем размер в строку
                hexString += " size[";
                hexString += std::to_string(byteArray.size());
                hexString += "]";

                return hexString;
            }
            case Type::StringArray:
            {
                const auto& byteArray = *std::static_pointer_cast<std::vector<uint8_t>>(value);
                std::string hexString;
                hexString += "[STRING-ARRAY] size[";
                hexString += std::to_string(byteArray.size());
                hexString += "]";
                return hexString;
            }
            case Type::Array:
            {
                const DeserializedValue& param = *std::static_pointer_cast<DeserializedValue> (value);
                std::string hex = "";
                hex += "[TYPE:";
                hex += std::to_string(static_cast<int>(param.type));
                hex += "]";
                switch (param.type) {
                case Type::Integer:
                    hex += "Integer size[" + std::to_string((*std::static_pointer_cast<std::vector<int32_t>>(param.value)).size()) + "]";
                    return hex;
                case Type::Long:
                    hex += "Long size[" + std::to_string((*std::static_pointer_cast<std::vector<int64_t>>(param.value)).size()) + "]";
                    return hex;
                case Type::Float:
                    hex += "Float size[" + std::to_string((*std::static_pointer_cast<std::vector<float>>(param.value)).size()) + "]";
                    return hex;
                case Type::Byte:
                    hex += "Byte size[" + std::to_string((*std::static_pointer_cast<std::vector<uint8_t>>(param.value)).size()) + "]";
                    return hex;
                case Type::Short: 
                    hex += "Short size[" + std::to_string((*std::static_pointer_cast<std::vector<int16_t>>(param.value)).size()) + "]";
                    return hex;
                case Type::String:
                    hex += "String size[" + std::to_string((*std::static_pointer_cast<std::vector<std::string>>(param.value)).size()) + "]";
                    return hex;
                default:
                    return "Unknown type";
                }
            
            }
            default:
                //throw std::runtime_error("Unknown type.");

                //std::cout << "Unknown type" << std::endl;
            {
                std::cout << " Value is null";
                return std::string();
            }
        }
    }

    // Функция для получения строки по значению Type
    const char* getTypeStr() {
        switch (type) {
        case Type::Unknown: return "Unknown";
        case Type::Null: return "Null";
        case Type::Dictionary: return "Dictionary";
        case Type::StringArray: return "StringArray";
        case Type::Byte: return "Byte";
        case Type::Double: return "Double";
        case Type::EventData: return "EventData";
        case Type::Float: return "Float";
        case Type::Integer: return "Integer";
        case Type::Hashtable: return "Hashtable";
        case Type::Short: return "Short";
        case Type::Long: return "Long";
        case Type::IntegerArray: return "IntegerArray";
        case Type::Boolean: return "Boolean";
        case Type::OperationResponse: return "OperationResponse";
        case Type::OperationRequest: return "OperationRequest";
        case Type::String: return "String";
        case Type::ByteArray: return "ByteArray";
        case Type::Array: return "Array";
        case Type::ObjectArray: return "ObjectArray";
        default: return "Unknown";  // На случай если тип не совпадает
        }
    }

};




class OperationResponse;
class OperationRequest;
class EventData;


class APACKETLIB_API Deserializer {
public:
    static DeserializedValue Deserialize(Stream& input);
    static DeserializedValue Deserialize(Stream& input, uint8_t typeCode);
    static std::vector<std::pair<uint8_t, DeserializedValue>> DeserializeParameterTable(Stream& input);

    static void test();


    static OperationResponse DeserializeOperationResponse(Stream& input);
    static OperationRequest DeserializeOperationRequest(Stream& input);
    static EventData DeserializeEventData(Stream& input);

    
private:
    static DeserializedValue DeseriallizeArray(Stream& stream);
    static uint8_t DeserializeByte(Stream& stream);
    static bool DeserializeBoolean(Stream& stream);
    static int16_t DeserializeShort(Stream& stream);
    static int32_t DeserializeInteger(Stream& stream);
    static int64_t DeserializeLong(Stream& stream);
    static float DeserializeFloat(Stream& stream);
    static double DeserializeDouble(Stream& stream);
    static std::string DeserializeString(Stream& stream);
    static std::vector<uint8_t> DeserializeByteArray(Stream& stream);
    static std::vector<int32_t> DeserializeIntArray(Stream& stream);
    static std::vector<std::string> DeserializeStringArray(Stream& stream);
    static std::map<uint8_t, DeserializedValue> DeserializeDictionary(Stream& stream);

    
};