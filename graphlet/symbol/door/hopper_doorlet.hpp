#pragma once
#pragma warning(disable: 4250)

#include "graphlet/primitive.hpp"

namespace WarGrey::SCADA {
	private enum class DoorState { Default, Open, Opening, Closed, Closing, Disabled, _ };

	private struct DoorStyle {
		Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ border_color;
		Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ border_hlcolor;
		Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ body_color;
		Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ door_color;
		Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ skeleton_color;
		Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ depth_color;
		Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ disable_color;
	};

	private class IHopperDoorlet
		: public WarGrey::SCADA::ISymbollet<WarGrey::SCADA::DoorState, WarGrey::SCADA::DoorStyle>
		, public WarGrey::SCADA::IRangelet<double> {
	public:
		IHopperDoorlet(WarGrey::SCADA::DoorState default_state, float radius, double degrees);
	};

	private class HopperDoorlet : public WarGrey::SCADA::IHopperDoorlet {
	public:
		HopperDoorlet(WarGrey::SCADA::DoorState default_state, float radius, double degrees = 0.0);
		HopperDoorlet(float radius, double degrees = 0.0);

	public:
		void construct() override;
		void update(long long count, long long interval, long long uptime) override;
		void draw(Microsoft::Graphics::Canvas::CanvasDrawingSession^ ds, float x, float y, float Width, float Height) override;

	protected:
		void prepare_style(WarGrey::SCADA::DoorState status, WarGrey::SCADA::DoorStyle& style) override;
		void on_state_changed(WarGrey::SCADA::DoorState state) override;
		void on_value_changed(double t) override;

	private:
		Microsoft::Graphics::Canvas::Geometry::CanvasCachedGeometry^ disable_line;
		Microsoft::Graphics::Canvas::Geometry::CanvasCachedGeometry^ depth_line;
		Microsoft::Graphics::Canvas::Geometry::CanvasGeometry^ door_partitions[3];

	private:
		bool flashing;
	};

	private class UpperHopperDoorlet : public IHopperDoorlet {
	public:
		UpperHopperDoorlet(WarGrey::SCADA::DoorState default_state, float radius, double degrees = 0.0);
		UpperHopperDoorlet(float radius, double degrees = 0.0);

	public:
		void construct() override;
		void fill_margin(float x, float y, float* top = nullptr, float* right = nullptr, float* bottom = nullptr, float* left = nullptr) override;
		void update(long long count, long long interval, long long uptime) override;
		void draw(Microsoft::Graphics::Canvas::CanvasDrawingSession^ ds, float x, float y, float Width, float Height) override;

	protected:
		void prepare_style(WarGrey::SCADA::DoorState status, WarGrey::SCADA::DoorStyle& style) override;
		void on_state_changed(WarGrey::SCADA::DoorState state) override;
		void on_value_changed(double t) override;

	private:
		Microsoft::Graphics::Canvas::Geometry::CanvasCachedGeometry^ disable_line;
		Microsoft::Graphics::Canvas::Geometry::CanvasCachedGeometry^ body;
		Microsoft::Graphics::Canvas::Geometry::CanvasGeometry^ border;
		Microsoft::Graphics::Canvas::Geometry::CanvasGeometry^ door;

	private:
		float brdiff;

	private:
		bool flashing;
	};

	private class Doorlet : public WarGrey::SCADA::IRangelet<double> {
	public:
		Doorlet(float width, float height = 0.0F, float thickness = 2.0F,
			Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ color = nullptr,
			Microsoft::Graphics::Canvas::Brushes::CanvasSolidColorBrush^ border_color = nullptr,
			Microsoft::Graphics::Canvas::Brushes::CanvasSolidColorBrush^ bottom_color = nullptr);

	public:
		void construct() override;
		void fill_extent(float x, float y, float* w = nullptr, float* h = nullptr) override;
		void draw(Microsoft::Graphics::Canvas::CanvasDrawingSession^ ds, float x, float y, float Width, float Height) override;

	protected:
		void on_value_changed(double t) override;

	private:
		Microsoft::Graphics::Canvas::Geometry::CanvasCachedGeometry^ progress;
		Microsoft::Graphics::Canvas::Geometry::CanvasCachedGeometry^ top;
		Microsoft::Graphics::Canvas::Geometry::CanvasCachedGeometry^ bottom;
		Microsoft::Graphics::Canvas::Geometry::CanvasCachedGeometry^ base;
		Microsoft::Graphics::Canvas::Geometry::CanvasGeometry^ body;
		Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ border_color;
		Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ bottom_color;
		Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ color;

	private:
		float width;
		float height;
		float thickness;
	};
}
