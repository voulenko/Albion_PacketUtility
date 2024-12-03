#include "BaseHandler.h"
#include "imGuiRenderer.h"

class DebugRequestHandler : public DebugHandler {
public:
	DebugRequestHandler() : DebugHandler() {}

	void handle(int code, const std::vector<std::pair<uint8_t, DeserializedValue>>& parameters) override {
		std::cout << "[DEBUG] Request Handler handled" << std::endl;

		ImGuiRenderer::test.push_back(data{ "00:00:00", "Request", code, parameters });
	}
};