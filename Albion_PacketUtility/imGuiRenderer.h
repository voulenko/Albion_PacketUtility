#pragma once
#include "ImGuiManager.h"
#include <list> 
#include <string>
#include <vector>
#include <Deserializer.h>
struct data {
	std::string time;
	std::string packet;
	int code;
	std::vector<std::pair<uint8_t, DeserializedValue>> parameters;
};

class ImGuiRenderer {
public:
	static std::vector<data> test;
	static void render();
	static void doRender();
	static bool done;
};