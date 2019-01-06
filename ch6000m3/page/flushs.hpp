#pragma once

#include "planet.hpp"
#include "satellite.hpp"
#include "plc.hpp"

#include "decorator/grid.hpp"

#include "graphlet/textlet.hpp"
#include "graphlet/statuslet.hpp"

namespace WarGrey::SCADA {
	private class FlushsPage : public WarGrey::SCADA::Planet {
	public:
		~FlushsPage() noexcept;
		FlushsPage(WarGrey::SCADA::PLCMaster* plc);

	public:
		void load(Microsoft::Graphics::Canvas::UI::CanvasCreateResourcesReason reason, float width, float height) override;
		void reflow(float width, float height) override;

	public:
		bool can_select(IGraphlet* g) override;
		bool can_select_multiple() override;
		void on_tap_selected(IGraphlet* g, float x, float y) override;
		void on_gesture(std::list<Windows::Foundation::Numerics::float2>& anchors, float x, float y) override;
		
	private:
		WarGrey::SCADA::PLCMaster* device;
		WarGrey::SCADA::PLCConfirmation* dashboard;
		WarGrey::SCADA::ISatellite* diagnostics;
		Windows::UI::Xaml::Controls::MenuFlyout^ gate_valve_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ upper_door_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ ps_pump_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ sb_pump_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ ps_ps_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ ps_sb_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ ps_2_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ sb_ps_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ sb_sb_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ sb_2_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ s2_ps_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ s2_sb_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ s2_2_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ p2_2_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ i2_2_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ ps_h_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ sb_h_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ s2_h_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ p2_h_op;

	private: // never deletes these graphlets mannually
		WarGrey::SCADA::Statusbarlet* statusbar;
		WarGrey::SCADA::Statuslinelet* statusline;

	private:
		WarGrey::SCADA::GridDecorator* grid;
	};
}
