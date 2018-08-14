﻿#include "graphlet/dashboard/thermometerlet.hpp"

#include "shape.hpp"
#include "paint.hpp"
#include "hatch.hpp"
#include "geometry.hpp"
#include "transformation.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::Foundation;
using namespace Windows::Foundation::Numerics;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::Brushes;
using namespace Microsoft::Graphics::Canvas::Geometry;

static CanvasSolidColorBrush^ thermometer_default_border_color = Colours::make(0xBBBBBB);
static unsigned int thermometer_default_colors[] = { 0x1E90FF, 0xB3F000, 0xFFB03A, 0xFFB03A };
static float thermometer_default_color_positions[] = { 0.0F, 0.625F, 0.75F, 1.0F };

static CanvasGeometry^ make_thermometer_glass(float width, float height, float thickness) {
	CanvasPathBuilder^ glass = ref new CanvasPathBuilder(CanvasDevice::GetSharedDevice());
	float offset = thickness * 0.5F;
	float bradius = (width - thickness) * 0.5F;
	float bulb_width = bradius * 2.0F;
	float tube_width = bulb_width * 0.618F;
	float tradius = tube_width * 0.5F;
	float tube_lx = (bulb_width - tube_width) * 0.5F + offset;
	float tube_ty = tradius + offset;
	float tube_rx = bulb_width - tube_lx + thickness;
	float tube_by = height - bradius * (std::sinf(std::acosf(tradius / bradius)) + 1.0F) - offset;

	glass->BeginFigure(tube_rx, tube_ty);
	glass->AddArc(float2(tube_lx, tube_ty), tradius, tradius, 0.0F, CanvasSweepDirection::CounterClockwise, CanvasArcSize::Small);
	glass->AddLine(tube_lx, tube_by);
	glass->AddArc(float2(tube_rx, tube_by), bradius, bradius, 0.0F, CanvasSweepDirection::CounterClockwise, CanvasArcSize::Large);
	glass->AddLine(tube_rx, tube_ty);
	glass->EndFigure(CanvasFigureLoop::Closed);

	return geometry_stroke(CanvasGeometry::CreatePath(glass), thickness);
}

static CanvasGeometry^ make_thermometer_mercury(float bulb_width, float height) {
	float bulb_radius = bulb_width * 0.5F;
	float cx = bulb_radius;
	float cy = height - bulb_radius;
	float tube_width = bulb_width * (1.0F - 0.618F);
	float tube_x = (bulb_width - tube_width) * 0.5F;
	float tradius = tube_width * 0.5F;
	
	CanvasGeometry^ bulb = circle(cx, cy, bulb_radius);
	CanvasGeometry^ bulb_hollow = circle(cx, cy, tradius);
	CanvasGeometry^ glass = rounded_rectangle(tube_x, 0.0F, tube_width, cy - tradius, tradius, tradius);

	return geometry_subtract(geometry_union(glass, bulb), bulb_hollow);
}

/*************************************************************************************************/
Thermometerlet::Thermometerlet(float width, float height, ICanvasBrush^ bcolor, GradientStops^ stops)
	: Thermometerlet(-30.0F, 50.0F, width, height, bcolor, stops) {}

Thermometerlet::Thermometerlet(float range, float width, float height, ICanvasBrush^ bcolor, GradientStops^ stops)
	: Thermometerlet(0.0F, range, width, height, bcolor, stops) {}

Thermometerlet::Thermometerlet(float tmin, float tmax, float width, float height, ICanvasBrush^ bcolor, GradientStops^ stops)
	: IRangelet(tmin,tmax), width(width), height(height), thickness(3.0F), border_color(bcolor) {
	if (this->height < 0.0F) {
		this->height *= (-this->width);
	} else if (this->height == 0.0F) {
		this->height = this->width * 2.0F;
	}

	this->colors = ((stops == nullptr) ? make_gradient_stops(thermometer_default_colors, thermometer_default_color_positions) : stops);
}

void Thermometerlet::construct() {
	float mercury_lowest, mercury_highest, mercury_height, em;
	float hatch_thickness = this->thickness * 0.618F;
	float hatch_width = vhatchmark_width(this->vmin, this->vmax, hatch_thickness, nullptr, nullptr, &em);
	
	this->bulb_size = this->width - hatch_width;
	this->fill_mercury_extent(0.0F, nullptr, &mercury_lowest);
	this->fill_mercury_extent(nullptr, &mercury_highest, nullptr, &mercury_height); 
	
	{ // make skeleton
		Rect hm_box;
		float hatch_height = (mercury_lowest - mercury_highest) + em;
		CanvasGeometry^ glass = make_thermometer_glass(this->bulb_size, this->height, this->thickness);
		CanvasGeometry^ hatch = vlhatchmark(hatch_height, this->vmin, this->vmax, hatch_thickness, &hm_box);
		float hatch_y = mercury_highest - hm_box.Y * 0.5F; // TODO: it is not very precision.

		glass = glass->Transform(make_translation_matrix(hatch_width, 0.0F));
		this->skeleton = geometry_freeze(geometry_union(glass, hatch, 0.0F, hatch_y));
	}

	this->on_value_changed(0.0F);
}

void Thermometerlet::fill_extent(float x, float y, float* w, float* h) {
	SET_VALUES(w, this->width, h, this->height);
}

void Thermometerlet::fill_mercury_extent(float percentage, float* x, float* y, float *width, float* height) {
	float mercury_width = this->bulb_size * 0.5F;
	float bulb_cx = this->width - mercury_width;
	float bulb_cy = this->height - mercury_width;
	float mercury_radius = mercury_width * 0.5F;
	float mercury_bulb_bottom = bulb_cy + mercury_radius;
	float mercury_highest = mercury_radius;
	float mercury_lowest = this->height - this->bulb_size;
	float mercury_tube_height = mercury_lowest - mercury_highest;
	float mercury_height = (mercury_bulb_bottom - mercury_lowest) + mercury_tube_height * percentage;
	float mercury_x = bulb_cx - mercury_radius;
	float mercury_y = mercury_bulb_bottom - mercury_height;

	SET_VALUES(x, mercury_x, y, mercury_y);
	SET_VALUES(width, mercury_width, height, mercury_height);
}

void Thermometerlet::fill_mercury_extent(float* x, float* y, float *width, float* height) {
	this->fill_mercury_extent(1.0F, x, y, width, height);
}

void Thermometerlet::on_value_changed(float v) {
	float mercury_width, mercury_height;
	float p = this->get_percentage();
	
	this->fill_mercury_extent(p, &this->mercury_x, &this->mercury_y, &mercury_width, &mercury_height);
	this->mercury = geometry_freeze(make_thermometer_mercury(mercury_width, mercury_height));
	this->mercury_color = make_solid_brush(gradient_discrete_color(this->colors, p));
}

void Thermometerlet::draw(CanvasDrawingSession^ ds, float x, float y, float Width, float Height) {
	ds->DrawCachedGeometry(this->mercury, x + this->mercury_x, y + this->mercury_y, this->mercury_color);
	ds->DrawCachedGeometry(this->skeleton, x, y, this->border_color);
}
