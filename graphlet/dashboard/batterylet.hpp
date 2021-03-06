#pragma once

#include "graphlet/primitive.hpp"
#include "paint.hpp"
#include "brushes.hxx"

namespace WarGrey::SCADA {
	private class Batterylet : public WarGrey::SCADA::IRangelet<double> {
	public:
		Batterylet(float width, float height, float thickness,
			Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ border_color = nullptr,
			WarGrey::SCADA::GradientStops^ stops = nullptr);

		Batterylet(double range, float width, float height, float thickness,
			Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ border_color = nullptr,
			WarGrey::SCADA::GradientStops^ stops = nullptr);

		Batterylet(double emin, double emax, float width, float height, float thickness,
			Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ border_color = nullptr,
			WarGrey::SCADA::GradientStops^ stops = nullptr);

	public:
		void construct() override;
		void fill_extent(float x, float y, float* w = nullptr, float* h = nullptr) override;
		void draw(Microsoft::Graphics::Canvas::CanvasDrawingSession^ ds, float x, float y, float Width, float Height) override;

	protected:
		void on_value_changed(double v) override;

	private:
		WarGrey::SCADA::GradientStops^ colors;
		Microsoft::Graphics::Canvas::Geometry::CanvasCachedGeometry^ skeleton;
		Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ border_color;
		Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ charge_color;

	private:
		Windows::Foundation::Rect charge;
		float width;
		float height;
		float thickness;
	};

	private class SystemBatterylet : public WarGrey::SCADA::Batterylet {
	public:
		SystemBatterylet(float width, float height,
			Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ border_color = nullptr,
			WarGrey::SCADA::GradientStops^ stops = nullptr);

	public:
		void update(long long count, long long interval, long long uptime) override;
	};
}
