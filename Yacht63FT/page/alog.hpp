#pragma once

#include "planet.hpp"
#include "plc.hpp"

namespace WarGrey::SCADA {
	private class ALogPage : public WarGrey::SCADA::Planet {
	public:
		virtual ~ALogPage() noexcept;

		ALogPage(PLCMaster* device, Platform::String^ name);

	public:
		void load(Microsoft::Graphics::Canvas::UI::CanvasCreateResourcesReason reason, float width, float height) override;
		void on_elapse(long long count, long long interval, long long uptime) override;
		void on_tap(IGraphlet* g, float local_x, float local_y, bool shifted, bool controlled) override;

	private:
		WarGrey::SCADA::PLCConfirmation* dashboard;
	};
}
