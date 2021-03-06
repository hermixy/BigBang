#pragma once

#include "graphlet/primitive.hpp"
#include "paint.hpp"
#include "brushes.hxx"

namespace WarGrey::SCADA {
	private class Indicatorlet : public WarGrey::SCADA::IRangelet<double> {
	public:
		Indicatorlet(float size, float thickness,
			Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ bgcolor = nullptr,
			WarGrey::SCADA::GradientStops^ stops = nullptr);

		Indicatorlet(double range, float size, float thickness,
			Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ bgcolor = nullptr,
			WarGrey::SCADA::GradientStops^ stops = nullptr);

		Indicatorlet(double vmin, double vmax, float size, float thickness,
			Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ bgcolor = WarGrey::SCADA::Colours::make(0x505050),
			WarGrey::SCADA::GradientStops^ stops = nullptr);

	public:
		void construct() override;
		void fill_extent(float x, float y, float* w = nullptr, float* h = nullptr) override;
		void fill_margin(float x, float y, float* t = nullptr, float* r = nullptr, float* b = nullptr, float* l = nullptr) override;
		void draw(Microsoft::Graphics::Canvas::CanvasDrawingSession^ ds, float x, float y, float Width, float Height) override;

	protected:
		void on_value_changed(double v) override;

	private:
		WarGrey::SCADA::GradientStops^ colors;
		Microsoft::Graphics::Canvas::Geometry::CanvasCachedGeometry^ body_ring;
		Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ bgcolor;
		Microsoft::Graphics::Canvas::Brushes::ICanvasBrush^ fgcolor;

	private:
		float bspace;
		float size;
		float thickness;
	};
}
