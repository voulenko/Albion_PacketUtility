#include "BaseHandler.h"
#include "imGuiRenderer.h"
#include <thread>

class DebugResponseHandler : public DebugHandler {
public:
	DebugResponseHandler() : DebugHandler() {}

	void handle(int code, const std::vector<std::pair<uint8_t, DeserializedValue>>& parameters) override {
		std::cout << "[DEBUG] Response Handler handled" << std::endl;

        auto now = std::chrono::system_clock::now();
        std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

        // ѕреобразуем в строку
        char timeBuffer[100];
        std::tm timeInfo;
        // »спользуем localtime_s дл€ безопасного преобразовани€ времени
        localtime_s(&timeInfo, &currentTime);  // safe localtime_s instead of unsafe localtime
        std::strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", &timeInfo);
        ImGuiRenderer::log.AddLog("[%s][RESPONSE] parsed %d parameters code %d\n", timeBuffer, parameters.size(), code);
		ImGuiRenderer::test.push_back(data{ timeBuffer, "Response", code, parameters });
	}
};