#include "BaseHandler.h"
#include "imGuiRenderer.h"

class DebugResponseHandler : public DebugHandler {
public:
	DebugResponseHandler() : DebugHandler() {}

	void handle(int code, const std::vector<std::pair<uint8_t, DeserializedValue>>& parameters) override {
		std::cout << "[DEBUG] Response Handler handled" << std::endl;

		ImGuiRenderer::test.push_back(data{ "00:00:00", "Response", code, parameters });
	}
};