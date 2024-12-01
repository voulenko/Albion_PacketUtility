#pragma once
#include "ImGuiManager.h"
#include <list> 
class ImGuiRenderer {
public:
	static void render();
	static void doRender();
	static bool done;
};