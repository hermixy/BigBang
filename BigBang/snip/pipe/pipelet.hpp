#pragma once

#include "snip/pipe/pipesnip.hpp"

namespace WarGrey::SCADA {
    private class Pipelet : public WarGrey::SCADA::IPipeSnip {
    public:
        Pipelet(float width, float height = 0.0F, float thickness = 0.0F,
            double color = nan("Silver"), double saturation = 0.0,
            double light = 0.512, double highlight = 0.753);

    public:
        void load() override;
        void update(long long count, long long interval, long long uptime, bool is_slow) override;
        void draw(Microsoft::Graphics::Canvas::CanvasDrawingSession^ ds, float x, float y, float Width, float Height) override;
        void fill_extent(float x, float y, float* w = nullptr, float* h = nullptr,
            float* d = nullptr, float* s = nullptr, float* l = nullptr, float* r = nullptr)
            override;

    public:
        Windows::Foundation::Rect get_input_port() override;
        Windows::Foundation::Rect get_output_port() override;

    private:
        float width;
        float height;
        float thickness;
        float connector_width;

    private:
        Windows::UI::Color color;
        Windows::UI::Color connector_color;
        Windows::UI::Color highlight_color;
        Microsoft::Graphics::Canvas::Geometry::CanvasCachedGeometry^ connector;
        Microsoft::Graphics::Canvas::Geometry::CanvasCachedGeometry^ hollow_body;
        Microsoft::Graphics::Canvas::Geometry::CanvasGeometry^ body_mask;
        Microsoft::Graphics::Canvas::Geometry::CanvasStrokeStyle^ cartoon_style;
        Microsoft::Graphics::Canvas::Brushes::CanvasLinearGradientBrush^ connector_brush;
        Microsoft::Graphics::Canvas::Brushes::CanvasLinearGradientBrush^ brush;
    };
}