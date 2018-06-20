﻿#include "graphlet/dashboard/batterylet.hpp"

#include "shape.hpp"
#include "geometry.hpp"
#include "system.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::Foundation;
using namespace Windows::Foundation::Numerics;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::Brushes;
using namespace Microsoft::Graphics::Canvas::Geometry;

static unsigned int battery_default_colors[] = {
	0xF00D0D,
	0xFFB33C,
	0xB4F100, 0xB4F100, 0xB4F100, 0xB4F100, 0xB4F100, 0xB4F100, 0xB4F100, 0xB4F100
};

private class BatteryStatus final : public ISystemStatusListener {
	friend class WarGrey::SCADA::Batterylet;
public:
	void on_battery_capacity_changed(float capacity) {
		this->capacity = capacity;
	}

private:
	float capacity;
};

static BatteryStatus* battery_status = nullptr;

/*************************************************************************************************/
Batterylet::Batterylet(float width, float height, ICanvasBrush^ bcolor, GradientStops^ stops)
	: Batterylet(0.0F, 1.0F, width, height, bcolor, stops) {}

Batterylet::Batterylet(float emin, float emax, float width, float height, ICanvasBrush^ bcolor, GradientStops^ stops)
	: IRangelet(emin, emax), width(width), height(height), thickness(this->width * 0.0618F), border_color(bcolor) {
	if (this->height < 0.0F) {
		this->height *= (-this->width);
	} else if (this->height == 0.0F) {
		this->height = this->width * 1.618F;
	}

	if (battery_status == nullptr) {
		battery_status = new BatteryStatus();
		register_system_status_listener(battery_status);
	}

	this->color_stops = ((stops == nullptr) ? make_gradient_stops(battery_default_colors) : stops);
}

void Batterylet::construct() {
	float corner_radius = this->thickness * 0.5F;
	float battery_y = this->height * 0.1F;
	float base_height = this->thickness * 1.618F;
	float battery_height = this->height - battery_y - base_height * 1.8F;
	float electrode_width = this->thickness * 1.618F;
	float electrode_center_y = this->height * 0.28F;
	float anode_center_x = this->width * 0.75F;
	float anode_x = anode_center_x - electrode_width * 0.5F;
	float anode_sidesize = this->width * 0.15F;
	float anode_y = electrode_center_y - anode_sidesize * 0.5F;
	float anode_hsymbol_x = anode_center_x - anode_sidesize * 0.5F;
	float anode_hsymbol_y = electrode_center_y - this->thickness * 0.5F;
	float anode_vsymbol_x = anode_center_x - this->thickness * 0.5F;
	float anode_vsymbol_y = electrode_center_y - anode_sidesize * 0.5F;
	float cathode_center_x = this->width * 0.25F;
	float cathode_sidesize = this->width * 0.18F;
	float cathode_x = cathode_center_x - electrode_width * 0.5F;
	float cathode_symbol_x = cathode_center_x - cathode_sidesize * 0.5F;
	float cathode_symbol_y = electrode_center_y - this->thickness * 0.5F;
	
	this->electricity.X = this->thickness;
	this->electricity.Y = battery_y + this->thickness;
	this->electricity.Width = this->width - this->electricity.X * 2.0F;
	this->electricity.Height = battery_height - (this->electricity.Y - battery_y) * 2.0F;

	this->electricity_color = make_linear_gradient_brush(
		0.0F, this->electricity.Y + this->electricity.Height,
		0.0F, this->electricity.Height,
		this->color_stops);

	auto battery_region = rounded_rectangle(0.0F, battery_y, this->width, battery_height, corner_radius, corner_radius);
	auto electricity_region = rectangle(this->electricity);
	
	CanvasGeometry^ battery_parts[] = {
		geometry_subtract(battery_region, electricity_region),
		rectangle(anode_x, 0.0F, electrode_width, battery_y),
		rectangle(anode_hsymbol_x, anode_hsymbol_y, anode_sidesize, this->thickness),
		rectangle(anode_vsymbol_x, anode_vsymbol_y, this->thickness, anode_sidesize),
		rectangle(cathode_x, 0.0F, electrode_width, battery_y),
		rectangle(cathode_symbol_x, cathode_symbol_y, cathode_sidesize, this->thickness),
		rectangle(0.0F, this->height - base_height, this->width, base_height)
	};

	this->skeleton = geometry_freeze(geometry_union(battery_parts)); // don't mind, it's Visual Studio's fault
}

void Batterylet::fill_extent(float x, float y, float* w, float* h) {
	SET_VALUES(w, this->width, h, this->height);
}

void Batterylet::update(long long count, long long interval, long long uptime) {
	this->set_value(battery_status->capacity);
}

void Batterylet::draw(CanvasDrawingSession^ ds, float x, float y, float Width, float Height) {
	float capacity = this->get_percentage();
	float capacity_height = fmin(this->electricity.Height * capacity, this->electricity.Height);
	float capacity_x = x + this->electricity.X;
	float capacity_y = y + this->electricity.Y + this->electricity.Height - capacity_height;

	brush_translate(this->electricity_color, x, y);

	ds->FillRectangle(capacity_x - 1.0F, capacity_y - 1.0F,
		this->electricity.Width + 2.0F, capacity_height + 2.0F,
		this->electricity_color);

	ds->DrawCachedGeometry(this->skeleton, x, y, this->border_color);
}
