#include "BaseHandler.h"
#include "imGuiRenderer.h"

class DebugEventHandler : public DebugHandler {
public:
	DebugEventHandler() : DebugHandler() {}

	void handle(int code, const std::vector<std::pair<uint8_t, DeserializedValue>>& parameters) override {
		std::cout << "[DEBUG] Event Handler handled" << std::endl;

		ImGuiRenderer::test.push_back(data{ "00:00:00", "Event", code, parameters});

	}
};