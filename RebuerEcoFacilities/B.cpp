﻿#include "B.hpp"
#include "tongue.hpp"
#include "system.hpp"
#include "syslog.hpp"

#include "text.hpp"
#include "paint.hpp"

#include "decorator/grid.hpp"
#include "decorator/pipeline.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::Foundation;

using namespace Windows::UI;
using namespace Windows::UI::ViewManagement;
using namespace Windows::UI::Xaml::Controls;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI;
using namespace Microsoft::Graphics::Canvas::Text;
using namespace Microsoft::Graphics::Canvas::Brushes;

#define SNIPS_ARITY(a) sizeof(a) / sizeof(Snip*)

static inline Motorlet* load_motorlet(IUniverse* master, float width, double degree = 0.0) {
	Motorlet* motor = new Motorlet(width);
	master->insert(motor, degree);

	return motor;
}

static inline Gaugelet* load_gaugelet(IUniverse* master, Platform::String^ caption, int A, int RPM) {
	Gaugelet* gauge = new Gaugelet(caption, A, RPM);
	master->insert(gauge);

	return gauge;
}

static inline Scalelet* load_scalelet(IUniverse* master, Platform::String^ unit, Platform::String^ label, Platform::String^ subscript) {
	Scalelet* scale = new Scalelet(unit, label, subscript);
	master->insert(scale);

	return scale;
}

static inline LSleevelet* load_sleevelet(IUniverse* master, float length, float thickness, double hue, double saturation, double light, double highlight) {
	LSleevelet* pipe = new LSleevelet(length, 0.0F, thickness, hue, saturation, light, highlight);
	master->insert(pipe);

	return pipe;
}

static inline Liquidlet* load_water_pipe(IUniverse* master, float length, double degrees, ArrowPosition ap = ArrowPosition::End) {
	Liquidlet* waterpipe = new Liquidlet(length, ap, 209.60, 1.000, 0.559);
	master->insert(waterpipe, degrees);

	return waterpipe;
}

static inline Liquidlet* load_oil_pipe(IUniverse* master, float length, double degrees) {
	Liquidlet* oilpipe = new Liquidlet(length, ArrowPosition::Start, 38.825, 1.000, 0.500);
	master->insert(oilpipe, degrees);

	return oilpipe;
}

static inline void connect_pipes(IUniverse* master, IPipeSnip* prev, IPipeSnip* pipe, float* x, float* y, double fx = 0.5, double fy = 0.5) {
    pipe_connecting_position(prev, pipe, x, y, fx, fy);
    master->move_to(pipe, (*x), (*y));
}

void connect_motor(IUniverse* master, IMotorSnip* pipe, Motorlet* motor, Scalelet* scale, float x, float y, double fx = 1.0, double fy = 1.0) {
	// TODO: there must be a more elegant way to deal with rotated motors
	float motor_width, motor_height, scale_width, scale_height, yoff;
	Rect mport = pipe->get_motor_port();

	motor->fill_extent(0.0F, 0.0F, &motor_width, &motor_height);
	scale->fill_extent(0.0F, 0.0F, &scale_width, &scale_height);
	master->fill_snip_bound(motor, nullptr, &yoff, nullptr, nullptr);

	x = x + mport.X + (mport.Width - motor_width) * float(fx);
	y = y + mport.Y + (mport.Height - motor_height + yoff) * float(fy);
	master->move_to(motor, x, y);

	if (yoff == 0.0F) {
		x = x + (motor_width - scale_width) * 0.3F;
		master->move_to(scale, x, y + motor_height);
	} else {
		x = x + (motor_width - scale_width) * 0.5F;
		master->move_to(scale, x, y - scale_height + yoff);
	}
}

// WARNING: order matters, Desulphurizer, Cleaner and Mooney are also the anchors for water pipes 
private enum B { Desulphurizer = 0, Funnel, Cleaner, Mooney, Count };

private class BConsole : public ModbusConfirmation {
public:
	BConsole(BSegment* master, Platform::String^ caption, Platform::String^ plc)
		: bench(master), caption(caption), device(plc)
		, inaddr0(126), inaddrn(358), inaddrq(MODBUS_MAX_READ_REGISTERS) {};

public:
	void load_window_frame(float width, float height) {
		this->statusline = new Statuslinelet(Log::Debug);
		this->statusbar = new Statusbarlet(this->caption, this->device, this, this->statusline);

		this->bench->insert(this->statusbar);
		this->bench->insert(this->statusline);
	}

	void load_icons(float width, float height) {
		this->icons[0] = new StorageTanklet(80.0F);

		for (size_t i = 0; i < SNIPS_ARITY(this->icons) && this->icons[i] != nullptr; i++) {
			this->bench->insert(this->icons[i]);
		}
	}

	void load_gauges(float width, float height) {
		this->gauges[B::Desulphurizer] = load_gaugelet(this->bench, "mastermotor", 100, 100);
		this->gauges[B::Funnel] = load_gaugelet(this->bench, "feedmotor", 200, 100);
		this->gauges[B::Cleaner] = load_gaugelet(this->bench, "cleanmotor", 10, 20);
		this->gauges[B::Mooney] = load_gaugelet(this->bench, "slavemotor", 200, 100);
	}

	void load_workline(float width, float height) {
		size_t dc = SNIPS_ARITY(this->desulphurizers);
		size_t vc1 = SNIPS_ARITY(this->viscosities_1st);
		size_t vc2 = SNIPS_ARITY(this->viscosities_2nd);

		float pipe_length = width / float(dc + vc1 + vc2 + 6);
		float pipe_thickness = pipe_length * 0.25F;
		float funnel_width = pipe_length * 0.382F;
		float gearbox_length = pipe_length * 1.25F;
		float dgbox_height = pipe_length * 1.25F;
		float mgbox_height = pipe_length * 0.750F;
		float cleaner_height = pipe_length;

		this->dgearbox = new LGearboxlet(gearbox_length, dgbox_height, pipe_thickness);
		this->mgearbox = new LGearboxlet(gearbox_length, mgbox_height, pipe_thickness);
		this->cleaner = new GlueCleanerlet(pipe_length, cleaner_height, pipe_thickness);
		this->funnel = new Funnellet(funnel_width, 0.0F, 120.0, 0.7, 0.3, 0.84);
		this->vibrator = new Vibratorlet(pipe_thickness * 2.718F);

		this->bench->insert(this->dgearbox);
		this->bench->insert(this->funnel);

		for (size_t i = 0; i < dc; i++) {
			this->desulphurizers[i] = load_sleevelet(this->bench, pipe_length, pipe_thickness, nan("Silver"), 0.000, 0.512, 0.753);
		}

		this->bench->insert(this->cleaner);
		this->bench->insert(this->mgearbox);

		for (size_t i = 0; i < SNIPS_ARITY(this->viscosities_1st); i++) {
			this->viscosities_1st[i] = load_sleevelet(this->bench, pipe_length, pipe_thickness, 120.0, 0.607, 0.339, 0.839);
		}

		this->bench->insert(this->vibrator);

		for (size_t i = 0; i < SNIPS_ARITY(this->viscosities_2nd); i++) {
			this->viscosities_2nd[i] = load_sleevelet(this->bench, pipe_length, pipe_thickness, 120.0, 0.607, 0.339, 0.839);
		}

		{ // load motors
			this->motors[B::Funnel] = load_motorlet(this->bench, funnel_width, 90.0);
			this->motors[B::Desulphurizer] = load_motorlet(this->bench, dgbox_height * 0.618F);
			this->motors[B::Mooney] = load_motorlet(this->bench, mgbox_height);
			this->motors[B::Cleaner] = load_motorlet(this->bench, pipe_thickness, 90.0);

			for (unsigned int i = 0; i < B::Count; i++) {
				this->Tms[i] = load_scalelet(this->bench, "celsius", "temperature", nullptr);
			}
		}

		{ // load water and oil pipes
			this->water_pipes[B::Desulphurizer] = load_water_pipe(this->bench, pipe_length, 0.0, ArrowPosition::Start);
			this->water_pipes[B::Cleaner] = load_water_pipe(this->bench, pipe_length, 0.0);
			for (size_t i = 0; i < SNIPS_ARITY(this->water_pipes); i++) {
				if (this->water_pipes[i] == nullptr) {
					this->water_pipes[i] = load_water_pipe(this->bench, pipe_length, 90.0);
				}
			}

			this->oil_pipes[B::Desulphurizer] = load_oil_pipe(this->bench, pipe_length, 0.0);
			for (size_t i = 1; i < SNIPS_ARITY(this->oil_pipes); i++) {
				this->oil_pipes[i] = load_oil_pipe(this->bench, pipe_length, 90.0);
			}
		}

		{ // load labels
			this->load_scales_pt(this->gbscales, SNIPS_ARITY(this->gbscales) / 2);
			this->load_scales_ptt(this->dscales, dc);
			this->load_scales_ptt(this->vscales_1st, vc1);
		}
	}

public:
	void reflow_window_frame(float width, float height) {
		this->statusbar->fill_extent(0.0F, 0.0F, nullptr, &this->console_y);
		this->bench->move_to(this->statusline, 0.0F, height - console_y);
	}

	void reflow_icons(float width, float height) {
		float icon_gapsize = 64.0F;
		float icon_hmax = 0.0F;
		float icon_x = 0.0F;
		float icon_y = this->console_y * 1.618F;
		float icon_width, icon_height;

		for (size_t i = 0; i < SNIPS_ARITY(this->icons) && this->icons[i] != nullptr; i++) {
			this->icons[i]->fill_extent(icon_x, icon_y, nullptr, &icon_height);
			icon_hmax = max(icon_height, icon_hmax);
		}

		for (size_t i = 0; i < SNIPS_ARITY(this->icons) && this->icons[i] != nullptr; i++) {
			this->icons[i]->fill_extent(icon_x, icon_y, &icon_width, &icon_height);
			this->bench->move_to(this->icons[i], icon_x, icon_y + (icon_hmax - icon_height) * 0.5F);
			icon_x += (icon_width + icon_gapsize);
		}
	}

	void reflow_gauges(float width, float height) {
		float gauge_gapsize = 32.0F;
		float gauge_x = 0.0F;
		float gauge_y = 0.0F;
		float snip_width, snip_height;

		this->gauges[0]->fill_extent(gauge_x, gauge_y, nullptr, &snip_height);
		gauge_y = height - snip_height - console_y;
		for (size_t i = 0; i < SNIPS_ARITY(this->gauges); i++) {
			this->bench->move_to(this->gauges[i], gauge_x, gauge_y);
			this->gauges[i]->fill_extent(gauge_x, gauge_y, &snip_width);
			gauge_x += (snip_width + gauge_gapsize);
		}
	}

	void reflow_screwline(float width, float height) {
		float pipe_length, pipe_thickness, snip_width, snip_height;
		size_t gc = SNIPS_ARITY(this->gbscales) / 2;
		size_t dc = SNIPS_ARITY(this->desulphurizers);
		size_t vc1 = SNIPS_ARITY(this->viscosities_1st);
		size_t vc2 = SNIPS_ARITY(this->viscosities_2nd);

		this->desulphurizers[0]->fill_extent(0.0F, 0.0F, &pipe_length, &pipe_thickness);
		this->funnel->fill_extent(0.0F, 0.0F, &snip_width, &snip_height);

		float current_x = pipe_length * 2.0F;
		float current_y = (height - pipe_length * 3.14F) * 0.5F;
		this->bench->move_to(this->funnel, current_x, current_y);
		this->move_motor(B::Funnel, this->funnel, current_x, current_y, 0.5, 1.0);
		connect_pipes(this->bench, this->funnel, this->dgearbox, &current_x, &current_y, 0.2, 0.5);
		this->move_motor(B::Desulphurizer, this->dgearbox, current_x, current_y);
		
		connect_pipes(this->bench, this->dgearbox, this->desulphurizers[0], &current_x, &current_y);
		for (size_t i = 1; i < dc; i++) {
			connect_pipes(this->bench, this->desulphurizers[i - 1], this->desulphurizers[i], &current_x, &current_y);
		}

		connect_pipes(this->bench, this->desulphurizers[dc - 1], this->cleaner, &current_x, &current_y);
		this->move_motor(B::Cleaner, this->cleaner, current_x, current_y, 0.5, 1.0);
		connect_pipes(this->bench, this->cleaner, this->mgearbox, &current_x, &current_y, 0.2, 0.5);
		this->move_motor(B::Mooney, this->mgearbox, current_x, current_y);
		
		connect_pipes(this->bench, this->mgearbox, this->viscosities_1st[0], &current_x, &current_y);
		for (size_t i = 1; i < vc1; i++) {
			connect_pipes(this->bench, this->viscosities_1st[i - 1], this->viscosities_1st[i], &current_x, &current_y);
		}

		this->vibrator->fill_extent(0.0F, 0.0F, &snip_width, &snip_height);
		this->bench->move_to(this->vibrator, current_x + pipe_length, current_y + pipe_thickness - snip_height);
		
		current_x += (pipe_length + snip_width);
		this->bench->move_to(this->viscosities_2nd[0], current_x, current_y);
		for (size_t i = 1; i < vc2; i++) {
			connect_pipes(this->bench, this->viscosities_2nd[i - 1], this->viscosities_2nd[i], &current_x, &current_y);
		}

		{ // flow liguid pipes and labels, TODO: if there is a more elegant way to deal with this
			Rect dport = this->dgearbox->get_input_port();
			Rect mport = this->dgearbox->get_motor_port();
			Rect cport = this->cleaner->get_output_port();
			Rect pport = this->desulphurizers[0]->get_input_port();

			float pipe_x, pipe_y, scale_width, scale_height, scale_xoff;
			float liquid_xoff, liquid_yoff, liquid_width, liquid_height;

			this->bench->fill_snip_bound(this->desulphurizers[0], &pipe_x, &pipe_y, nullptr, nullptr);
			this->bench->fill_snip_bound(this->oil_pipes[1], nullptr, &liquid_yoff, nullptr, nullptr);
			this->dscales[0]->fill_extent(0.0F, 0.0F, &scale_width, &scale_height);
			this->water_pipes[0]->fill_extent(0.0F, 0.0F, &liquid_width, &liquid_height);

			current_y = pipe_y + pport.Y;
			scale_xoff = (pipe_length - scale_width) * 0.25F;
			liquid_xoff = (pipe_length - liquid_width) * 0.5F;
			liquid_yoff = pport.Y + liquid_yoff - liquid_height;
			this->move_scales_ptt(this->dscales, dc, pipe_x + scale_xoff, current_y, scale_height, pipe_length, pport.Height);
			for (size_t i = 0; i < SNIPS_ARITY(this->oil_pipes) - 1; i++) {
				this->bench->move_to(this->oil_pipes[i + 1], pipe_x + liquid_xoff + pipe_length * i, pipe_y + liquid_yoff);
			}

			liquid_yoff = liquid_yoff + pipe_thickness + liquid_width - pport.Y * 2.0F;
			this->bench->fill_snip_bound(this->dgearbox, &pipe_x, nullptr, nullptr, nullptr);
			current_x = pipe_x + mport.X - liquid_width;
			this->bench->move_to(this->oil_pipes[B::Desulphurizer], current_x, current_y);
			this->bench->move_to(this->water_pipes[B::Desulphurizer], current_x, current_y + liquid_height);
			this->move_scales_pt(this->gbscales, gc, 0, pipe_x + dport.X, current_y + pport.Height, scale_height);
			current_x = pipe_x + dport.X + (dport.Width - liquid_width) * 0.5F;
			current_y = pipe_y + (pipe_thickness - liquid_height) * 0.5F;
			
			this->bench->fill_snip_bound(this->cleaner, &pipe_x, &pipe_y, nullptr, nullptr);
			this->bench->move_to(this->water_pipes[B::Cleaner], pipe_x + cport.X + cport.Width, current_y + liquid_height);
			this->bench->move_to(this->water_pipes[1], current_x, pipe_y + dport.Y + liquid_yoff);
			this->move_scales_pt(this->gbscales, gc, 1, pipe_x + cport.X, pipe_y + cport.Y + pport.Height, scale_height);
			current_x = pipe_x + cport.X + (cport.Width - liquid_width) * 0.5F;
			this->bench->fill_snip_bound(this->viscosities_1st[0], &pipe_x, &pipe_y, nullptr, nullptr);
			current_y = pipe_y + liquid_yoff;
			this->bench->move_to(this->water_pipes[B::Mooney], current_x, current_y);
			for (size_t i = B::Mooney + 1; i < SNIPS_ARITY(this->water_pipes); i++) {
				this->bench->move_to(this->water_pipes[i], pipe_x + liquid_xoff + pipe_length * (i - B::Mooney - 1), current_y);
			}

			this->bench->fill_snip_bound(this->viscosities_1st[0], &pipe_x, &pipe_y, nullptr, nullptr);
			this->move_scales_ptt(this->vscales_1st, vc1, pipe_x + scale_xoff, pipe_y + pport.Y, scale_height, pipe_length, pport.Height);
		}
	}

public:
	void fill_application_input_register_interval(uint16* addr0, uint16* addrn, uint16* addrq) override {
		SET_VALUES(addr0, this->inaddr0, addrn, this->inaddrn);
		SET_BOX(addrq, this->inaddrq);
	}

	void on_input_registers(uint16 transaction, uint16 address, uint16* register_values, uint8 count, Syslog* logger) override {
		size_t dc = SNIPS_ARITY(this->desulphurizers);
		size_t vc1 = SNIPS_ARITY(this->viscosities_1st);
		uint16* modbus = register_values - address;

		if ((count != this->inaddrq) && (address + count != this->inaddrn)) {
			logger->log_message(Log::Warning,
				L"Job(%hu) done, but read less input registers, expected quantity: %d, given %d",
				transaction, min(this->inaddrn - address, this->inaddrq), count);
		}

		switch ((address - this->inaddr0) / this->inaddrq) {
		case 0: { // [126, 251)
			float ratio = 0.1F;

			this->gauges[B::Desulphurizer]->set_rpm(modbus[126]);
			this->gauges[B::Mooney]->set_rpm(modbus[127]);

			this->update_scales_pt(this->gbscales,     0,   modbus, 150, ratio);
			this->update_scales_ptt(this->dscales,     dc,  modbus, 152, ratio);
			this->update_scales_pt(this->gbscales,     2,   modbus, 166, ratio);
			this->update_scales_ptt(this->vscales_1st, vc1, modbus, 168, ratio);

			for (size_t wpi = 0; wpi < SNIPS_ARITY(this->water_pipes); wpi++) {
				size_t idx = 182 + wpi * 8;
				// inner water circulation
				float Ti = float(modbus[idx + 0] * ratio);
				float To = float(modbus[idx + 1] * ratio);

				this->water_pipes[wpi]->set_temperatures(Ti, To);
			}
		} break;
		default: { // [251, 325]; // 325 < 251 + 125(MODBUS_MAX_READ_REGISTERS)

		}
		}
	}

	void on_exception(uint16 transaction, uint8 function_code, uint16 maybe_address, uint8 reason, Syslog* logger) override {
		logger->log_message(Log::Error, L"Job(%hu, 0x%02X) failed due to reason %d", transaction, function_code, reason);
	};

private:
	void move_motor(B id, IMotorSnip* pipe, float x, float y, double fx = 1.0, double fy = 1.0) {
		connect_motor(this->bench, pipe, this->motors[id], this->Tms[id], x, y, fx, fy);
	}

	void load_scales_t(Scalelet* scales[], size_t arity) {
		for (size_t i = 0; i < arity; i++) {
			scales[i] = load_scalelet(this->bench, "celsius", "temperature", nullptr);
		}
	}

	void load_scales_pt(Scalelet* scales[], size_t arity) {
		for (size_t i = 0; i < arity; i++) {
			scales[i + arity * 0] = load_scalelet(this->bench, "bar", "pressure", nullptr);
			scales[i + arity * 1] = load_scalelet(this->bench, "celsius", "temperature", nullptr);
		}
	}

	void load_scales_ptt(Scalelet* scales[], size_t arity) {
		for (size_t i = 0; i < arity; i++) {
			scales[i + arity * 0] = load_scalelet(this->bench, "bar", "pressure", nullptr);
			scales[i + arity * 1] = load_scalelet(this->bench, "celsius", "temperature", "inside");
			scales[i + arity * 2] = load_scalelet(this->bench, "celsius", "temperature", "outside");
		}
	}

	void move_scales_pt(Scalelet* scales[], size_t arity, size_t idx, float x, float y, float scale_height) {
		this->bench->move_to(scales[idx + arity * 0], x, y);
		this->bench->move_to(scales[idx + arity * 1], x, y + scale_height);
	}

	void move_scales_ptt(Scalelet* scales[], size_t arity, float x0, float y_ascent, float scale_height, float hgap, float vgap) {
		float y_descent = y_ascent + vgap;

		for (size_t idx = 0; idx < arity; idx++) {
			float x = x0 + hgap * float(idx);

			this->bench->move_to(scales[idx + arity * 0], x, y_ascent - scale_height);
			this->bench->move_to(scales[idx + arity * 1], x, y_descent + scale_height * 0.0F);
			this->bench->move_to(scales[idx + arity * 2], x, y_descent + scale_height * 1.0F);
		}
	}

	void update_scales_pt(Scalelet* scales[], size_t i, uint16* modbus, size_t idx, float ratio) {
		scales[i + 0]->set_scale(float(modbus[idx + 0]) * ratio);
		scales[i + 1]->set_scale(float(modbus[idx + 1]) * ratio);
	}

	void update_scales_ptt(Scalelet* scales[], size_t arity, uint16* modbus, size_t addr, float ratio) {
		for (size_t i = 0; i < arity; i++) {
			size_t idx = addr + i * 3;
		
			scales[i + arity * 0]->set_scale(float(modbus[idx + 0] * ratio));
			scales[i + arity * 1]->set_scale(float(modbus[idx + 1] * ratio));
			scales[i + arity * 2]->set_scale(float(modbus[idx + 2] * ratio));
		}
	}

// never deletes these snips mannually
private:
	Statusbarlet* statusbar;
	Statuslinelet* statusline;
	Snip* icons[1];
	Gaugelet* gauges[B::Count];

private:
	Gearboxlet* dgearbox;
	Gearboxlet* mgearbox;
	GlueCleanerlet* cleaner;
	Scalelet* gbscales[2 * 2];
	Funnellet* funnel;
	Vibratorlet* vibrator;
	Sleevelet* desulphurizers[4];
	Scalelet* dscales[4 * 3];
	Sleevelet* viscosities_1st[2];
	Scalelet* vscales_1st[2 * 3];
	Sleevelet* viscosities_2nd[2];
	Motorlet* motors[B::Count];
	Scalelet* Tms[B::Count];
	Liquidlet* oil_pipes[5];
	Liquidlet* water_pipes[6];

private:
	Platform::String^ caption;
	Platform::String^ device;
	BSegment* bench;

private:
	float console_y;

private:
	uint16 inaddr0;
	uint16 inaddrn;
	uint16 inaddrq;
};

private class BConsoleDecorator : public IUniverseDecorator {
public:
	BConsoleDecorator(Platform::String^ caption, Color& caption_color, float fontsize) {
		auto font = make_text_format("Consolas", fontsize);

		this->color = make_solid_brush(caption_color);
		this->caption = make_text_layout(speak(caption), font);
	};

public:
	void draw_before(IUniverse* self, CanvasDrawingSession^ ds, float Width, float Height) {
		float x = (Width - this->caption->LayoutBounds.Width) * 0.5F;
		float y = this->caption->DrawBounds.Y - this->caption->LayoutBounds.Y;

		ds->DrawTextLayout(this->caption, x, y, this->color);
	}

	void draw_before_snip(Snip* self, CanvasDrawingSession^ ds, float x, float y, float width, float height) override {
		if (x == 0.0) {
			if (y == 0.0) { // statusbar's bottomline 
				ds->DrawLine(0, height, width, height, this->color, 2.0F);
			} else if (dynamic_cast<Statuslinelet*>(self) != nullptr) { // statusline's topline
				ds->DrawLine(0, y, width, y, this->color, 2.0F);
			}
		}
	}

private:
	ICanvasBrush^ color;
	CanvasTextLayout^ caption;
};

BSegment::BSegment(Panel^ parent, Platform::String^ label, Platform::String^ plc) : Universe(parent, 4) {
	this->console = new BConsole(this, "RR" + label, plc);
	this->set_decorator(new BConsoleDecorator(label, system_color(UIElementType::GrayText), 64.0F));
}

BSegment::~BSegment() {
	if (this->console != nullptr) {
		delete this->console;
	}
}

void BSegment::load(CanvasCreateResourcesEventArgs^ args, float width, float height) {
	auto console = dynamic_cast<BConsole*>(this->console);

	if (console != nullptr) {
		console->load_icons(width, height);
		console->load_gauges(width, height);
		console->load_workline(width, height);
		console->load_window_frame(width, height);
	}
}

void BSegment::reflow(float width, float height) {
	auto console = dynamic_cast<BConsole*>(this->console);

	if (console != nullptr) {
		console->reflow_window_frame(width, height);
		console->reflow_icons(width, height);
		console->reflow_gauges(width, height);
		console->reflow_screwline(width, height);
	}
}