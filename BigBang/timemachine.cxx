﻿#include <list>
#include <map>

#include "timemachine.hpp"
#include "planet.hpp"

#include "navigator/listview.hpp"

#include "graphlet/textlet.hpp"
#include "graphlet/statuslet.hpp"
#include "graphlet/time/datepickerlet.hpp"
#include "graphlet/time/timelinelet.hpp"

#include "path.hpp"
#include "string.hpp"
#include "module.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::Foundation;
using namespace Windows::Storage;

using namespace Windows::UI::Input;

using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;

using namespace Microsoft::Graphics::Canvas::UI;
using namespace Microsoft::Graphics::Canvas::Text;
using namespace Microsoft::Graphics::Canvas::Brushes;

private enum class TMIcon : unsigned int { PrintScreen, BookMark, Quit, _ };
private enum class TM : unsigned int { Departure, Destination, _ };

static const float time_machine_alpha = 0.64F;

/*************************************************************************************************/
private ref class TimeMachineDisplay sealed : public UniverseDisplay {
internal:
	TimeMachineDisplay(Syslog* logger, ITimeMachine* entity, IHeadUpPlanet* dashboard)
		: UniverseDisplay(logger, nullptr, new ListViewNavigator(), dashboard), machine(entity), closed(true) {
		this->use_global_mask_setting(false);

		this->_void = ref new SplitView();
		this->_void->Content = this->canvas;
		this->_void->Pane = this->navigator->user_interface();

		this->_void->OpenPaneLength = 256.0;
		this->_void->DisplayMode = SplitViewDisplayMode::Overlay;
	}

internal:
	void pickup_planet(IPlanet* planet) {
		UniverseDisplay::push_planet(planet);
	}

public:
	SplitView^ flyout_content() {
		return this->_void;
	}

public:
	void on_opening(Platform::Object^ target, Platform::Object^ args) {
		this->darkness = this->global_mask_alpha;
		this->machine->on_showing();
	}

	void on_opened(Platform::Object^ target, Platform::Object^ args) {
		this->closed = false;
		this->global_mask_alpha = 0.8;
		this->_void->IsPaneOpen = false;
		this->machine->on_shown();
	}

	void on_closing(FlyoutBase^ target, FlyoutBaseClosingEventArgs^ args) {
		args->Cancel = !(this->machine->can_hide());
	}

	void on_closed(Platform::Object^ target, Platform::Object^ args) {
		this->closed = true;
		this->machine->on_hiden();
		this->global_mask_alpha = this->darkness;
	}

	void on_elapse(long long count, long long interval, long long uptime) override {
		this->machine->step();
	}

public:
	bool shown() override {
		return !(this->closed);
	}

protected:
	void construct(CanvasCreateResourcesReason reason) override {
		this->machine->construct(reason);
	}

private: // never delete these objects manually
	ITimeMachine* machine;
	SplitView^ _void;
	double darkness;
	bool closed;
};

private class TimeMachineDashboard : public IHeadUpPlanet, public ITimeMachineListener, public ITimelineListener {
public:
	TimeMachineDashboard(ITimeMachine* master, int frame_rate) : IHeadUpPlanet(__MODULE__), machine(master), frame_rate(frame_rate) {}

	void fill_margin(float* top = nullptr, float* right = nullptr, float* bottom = nullptr, float* left = nullptr) override {
		float base_size = statusbar_height();

		SET_BOX(top, base_size * 3.0F);
		SET_BOX(bottom, base_size);
		SET_BOXES(left, right, base_size);
	}

public:
	void load(CanvasCreateResourcesReason reason, float width, float height) override {
		auto icon_font = make_text_format("Consolas", statusbar_height());
		auto icon_color = Colours::GhostWhite;

		for (TMIcon id = _E0(TMIcon); id < TMIcon::_; id++) {
			Platform::String^ caption = nullptr;

			switch (id) {
			case TMIcon::PrintScreen: caption = L"📸"; break;
			case TMIcon::BookMark: caption = L"🔖"; break;
			case TMIcon::Quit: caption = L"🚪"; break;
			}

			this->icons[id] = this->insert_one(new Credit<Labellet, TMIcon>(caption, icon_font, icon_color), id);
			this->cellophane(this->icons[id], time_machine_alpha);
		}

		this->load_date_picker(this->time_pickers, TM::Departure, -this->machine->span_seconds());
		this->load_date_picker(this->time_pickers, TM::Destination, 0LL);

		{ // load the timeline
			long long departure = this->time_pickers[TM::Departure]->get_value() * 1000LL;
			long long destination = this->time_pickers[TM::Destination]->get_value() * 1000LL;
			float tlwidth = width * 0.618F;

			this->timeline = this->insert_one(new Timelinelet(departure, destination, tlwidth, this->frame_rate));
			this->cellophane(this->timeline, time_machine_alpha);

			this->timeline->push_event_listener(this);
		}
	}

	void reflow(float width, float height) override {
		float icon_y = statusbar_height() * 0.25F;
		float gapsize = icon_y * 2.0F;
		float icon_rx = width - icon_y;
		float icon_width;

		for (unsigned int idx = _N(TMIcon); idx > 0; idx--) {
			auto icon = this->icons[_E(TMIcon, idx - 1)];

			icon->fill_extent(0.0F, 0.0F, &icon_width);
			this->move_to(icon, icon_rx, icon_y, GraphletAnchor::RT);

			icon_rx -= (icon_width + gapsize);
		}

		this->move_to(this->time_pickers[TM::Departure], icon_y, icon_y, GraphletAnchor::LT);
		this->move_to(this->time_pickers[TM::Destination], this->time_pickers[TM::Departure], GraphletAnchor::LB, GraphletAnchor::LT);
		this->move_to(this->timeline, this->time_pickers[TM::Destination], GraphletAnchor::RT, GraphletAnchor::LC, gapsize * 2.0F);
	}

public:
	bool can_select(IGraphlet* g) override {
		return ((dynamic_cast<Labellet*>(g) != nullptr)
			|| (dynamic_cast<DatePickerlet*>(g) != nullptr));
	}

	void on_focus(IGraphlet* g, bool yes) override {
		auto editor = dynamic_cast<DatePickerlet*>(g);

		if (editor != nullptr) {
			if (this->timeline->can_change_range()) {
				if (yes) {
					this->show_virtual_keyboard(ScreenKeyboard::Bucketpad, GraphletAnchor::CB, 0.0F, 4.0F);
				} else {
					long long departure = this->time_pickers[TM::Departure]->get_value() * 1000LL;
					long long destination = this->time_pickers[TM::Destination]->get_value() * 1000LL;

					this->machine->startover(departure, destination);
				}
			} else {
				this->set_caret_owner(nullptr);
			}
		}
	}

	void on_tap_selected(IGraphlet* g, float local_x, float local_y) override {
		auto icon = dynamic_cast<Credit<Labellet, TMIcon>*>(g);

		if (icon != nullptr) {
			switch (icon->id) {
			case TMIcon::Quit: this->machine->hide(); break;
			case TMIcon::PrintScreen: {
				auto tmd = dynamic_cast<TimeMachineDisplay^>(this->master());

				if (tmd != nullptr) {
					tmd->save(this->name() + "-"
						+ tmd->current_planet->name() + "-"
						+ file_basename_from_second(this->timeline->get_value()) + ".png");
				}
			}; break;
			case TMIcon::BookMark: {
				auto tmd = dynamic_cast<TimeMachineDisplay^>(this->master());

				if (tmd != nullptr) {
					tmd->flyout_content()->IsPaneOpen = true;
				}
			}; break;
			}
		}
	}

public:
	void on_step(Timelinelet* timeline) override {
		this->machine->step();
	}

	void on_time_skipped(Timelinelet* timeline, long long timepoint_ms) override {
		this->machine->timeskip(timepoint_ms);
	}

	void on_timestream(long long timepoint_ms, size_t addr0, size_t addrn, const char* data, size_t size, Syslog* logger) override {
		this->timeline->set_value(timepoint_ms);
	}

public:
	Timelinelet* get_timeline() {
		return this->timeline;
	}

private:
	void load_date_picker(std::map<TM, Credit<DatePickerlet, TM>*>& tps, TM id, long long time_offset) {
		tps[id] = this->insert_one(new Credit<DatePickerlet, TM>(DatePickerState::Input, time_offset, _speak(id)), id);
		this->cellophane(tps[id], time_machine_alpha);
	}

private: // never delete this graphlets manually
	std::map<TMIcon, Credit<Labellet, TMIcon>*> icons;
	std::map<TM, Credit<DatePickerlet, TM>*> time_pickers;
	Timelinelet* timeline;

private:
	ITimeMachine* machine;
	unsigned int frame_rate;
};

/*************************************************************************************************/
ITimeMachine::ITimeMachine(Platform::String^ dirname, long long time_speed_mspf, int frame_rate, Syslog* logger
	, Platform::String^ prefix, Platform::String^ suffix, RotationPeriod period, unsigned int period_count)
	: IRotativeDirectory(dirname, prefix, suffix, period, period_count), ms_per_frame(time_speed_mspf), timepoint(0LL) {
	TimeMachineDisplay^ _universe = ref new TimeMachineDisplay(logger, this, new TimeMachineDashboard(this, frame_rate));
	
	this->machine = ref new Flyout();
	this->machine->Content = _universe->flyout_content();
	this->machine->Placement = FlyoutPlacementMode::Full;
	
	this->machine->Opening += ref new EventHandler<Platform::Object^>(_universe, &TimeMachineDisplay::on_opening);
	this->machine->Opened += ref new EventHandler<Platform::Object^>(_universe, &TimeMachineDisplay::on_opened);
	this->machine->Closing += ref new TypedEventHandler<FlyoutBase^, FlyoutBaseClosingEventArgs^>(_universe, &TimeMachineDisplay::on_closing);
	this->machine->Closed += ref new EventHandler<Platform::Object^>(_universe, &TimeMachineDisplay::on_closed);

	this->universe = _universe;
	FlyoutBase::SetAttachedFlyout(this->universe->canvas, this->machine);
}

void ITimeMachine::pickup(ITimeMachineListener* passenger) {
	auto planet = dynamic_cast<IPlanet*>(passenger);
	
	this->passengers.push_back(passenger);

	if (planet != nullptr) {
		auto tmd = dynamic_cast<TimeMachineDisplay^>(this->universe);

		if (tmd != nullptr) {
			tmd->pickup_planet(planet);
		}
	}
}

void ITimeMachine::startover(long long departure_ms, long long destination_ms) {
	auto dashboard = dynamic_cast<TimeMachineDashboard*>(this->universe->heads_up_planet);

	if (dashboard != nullptr) {
		this->departure = departure_ms;
		this->timepoint = departure_ms;
		this->destination = destination_ms;

		if (this->departure < this->destination) {
			this->direction = 1LL;
			this->timepoint = this->departure;
		} else {
			this->direction = -1LL;
			this->timepoint = this->destination;
		}

		dashboard->get_timeline()->set_range(departure_ms, destination_ms);
	}
}

void ITimeMachine::travel() {
	auto dashboard = dynamic_cast<TimeMachineDashboard*>(this->universe->heads_up_planet);

	if (dashboard != nullptr) {
		dashboard->get_timeline()->set_state(TimelineState::Travel);
	}
}

void ITimeMachine::step() {
	if (this->timepoint <= this->destination) {
		this->on_timestream(this->timepoint, 0, 0, nullptr, 0, this->get_logger());
		this->timepoint += this->ms_per_frame;
	}
}

void ITimeMachine::service() {
	auto dashboard = dynamic_cast<TimeMachineDashboard*>(this->universe->heads_up_planet);

	if (dashboard != nullptr) {
		dashboard->get_timeline()->set_state(TimelineState::Service);
	}
}

void ITimeMachine::terminate() {
	auto dashboard = dynamic_cast<TimeMachineDashboard*>(this->universe->heads_up_planet);

	if (dashboard != nullptr) {
		dashboard->get_timeline()->set_state(TimelineState::Terminated);
	}
}

void ITimeMachine::shift_speed() {
	auto dashboard = dynamic_cast<TimeMachineDashboard*>(this->universe->heads_up_planet);

	if (dashboard != nullptr) {
		dashboard->get_timeline()->shift_speed();
	}
}

void ITimeMachine::timeskip(long long timepoint) {
	auto dashboard = dynamic_cast<TimeMachineDashboard*>(this->universe->heads_up_planet);

	this->timepoint = timepoint;

	if (dashboard != nullptr) {
		dashboard->get_timeline()->set_value(timepoint);
	}
}

/**************************************************************************************************/
void ITimeMachine::on_timestream(long long timepoint_ms, size_t addr0, size_t addrn, const char* data, size_t size, Syslog* logger) {
	auto dashboard = dynamic_cast<ITimeMachineListener*>(this->universe->heads_up_planet);

	if (dashboard != nullptr) {
		dashboard->on_timestream(timepoint_ms, addr0, addrn, data, size, logger);
	}

	for (auto passenger : this->passengers) {
		passenger->on_timestream(timepoint_ms, addr0, addrn, data, size, logger);
	}
}

unsigned int ITimeMachine::get_speed_shift() {
	auto dashboard = dynamic_cast<TimeMachineDashboard*>(this->universe->heads_up_planet);
	unsigned int shift = 1U;

	if (dashboard != nullptr) {
		shift = dashboard->get_timeline()->get_speed_shift();
	}

	return shift;
}

long long ITimeMachine::get_time_speed() {
	return this->ms_per_frame;
}

Syslog* ITimeMachine::get_logger() {
	return this->universe->get_logger();
}

Flyout^ ITimeMachine::user_interface() {
	return this->machine;
}

/*************************************************************************************************/
TimeMachine::TimeMachine(Platform::String^ dirname, long long time_speed_mspf, int frame_rate, Syslog* logger
	, Platform::String^ file_prefix, Platform::String^ file_suffix, RotationPeriod period, unsigned int period_count)
	: ITimeMachine(dirname, time_speed_mspf, frame_rate, logger, file_prefix, file_suffix, period, period_count) {}

void TimeMachine::on_file_rotated(StorageFile^ prev_file, StorageFile^ current_file, long long timepoint) {
	if (prev_file != nullptr) {
		this->tmstream.close();
	}
	
	// TODO: find the reason if `open` fails.
	this->tmstream.open(current_file->Path->Data(), std::ios::out | std::ios::app);
}

void TimeMachine::save_snapshot(long long timepoint_ms, size_t addr0, size_t addrn, const char* data, size_t size) {
	// TODO: find the reason if `write` fails.
	if (this->tmstream.is_open()) {
		this->tmstream << timepoint_ms << " " << addr0 << " " << addrn << std::endl;
		this->tmstream.write(data, size) << std::endl;
		this->tmstream.flush();
	}
}

const char* TimeMachine::seek_snapshot(long long* timepoint_ms, size_t* addr0, size_t* addrn) {
	return nullptr;
}
