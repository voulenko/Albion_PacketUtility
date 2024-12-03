#pragma once

#ifdef APACKETLIB_EXPORTS  // Этот макрос будет определен в проекте DLL
#define APACKETLIB_API __declspec(dllexport)
#else
#define APACKETLIB_API __declspec(dllimport)
#endif
#include <vector>

class APACKETLIB_API NumberSerializer {
public:
    static void Serialize(int32_t value, std::vector<uint8_t>& target, size_t& offset);
};
