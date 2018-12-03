﻿#include "schema/timeseries/earthwork_ds.hpp"
#include "schema/earthwork.hpp"
#include "dbmisc.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::Foundation;
using namespace Windows::System;

using namespace Microsoft::Graphics::Canvas::Brushes;

/*************************************************************************************************/
ICanvasBrush^ WarGrey::SCADA::earthwork_line_color_dictionary(unsigned int index) {
	ICanvasBrush^ color = nullptr;

	switch (_E(EWTS, index % _N(EWTS))) {
	case EWTS::EarthWork: color = Colours::Khaki; break;
	case EWTS::Vessel: color = Colours::Cyan; break;
	case EWTS::HopperHeight: color = Colours::Crimson; break;
	case EWTS::Loading: color = Colours::Orange; break;
	case EWTS::Displacement: color = Colours::MediumSeaGreen; break;
	}

	return color;
}

/*************************************************************************************************/
EarthWorkDataSource::EarthWorkDataSource(Syslog* logger, RotationPeriod period, unsigned int period_count)
	: RotativeSQLite3("earthwork", logger, period, period_count) {}

bool EarthWorkDataSource::ready() {
	return RotativeSQLite3::ready();
}

void EarthWorkDataSource::on_database_rotated(WarGrey::SCADA::SQLite3* prev_dbc, WarGrey::SCADA::SQLite3* dbc) {
	// TODO: move the temporary data from in-memory SQLite3 into the current SQLite3

	create_earthwork(dbc, true);
	this->get_logger()->log_message(Log::Warning, L"current file: %S", dbc->filename().c_str());
}

void EarthWorkDataSource::load(WarGrey::SCADA::ITimeSeriesDataReceiver* receiver, long long open_s, long long closed_s) {
	bool asc = (open_s < closed_s);
	long long timepoint = open_s;

	do {
		Platform::String^ dbsource = this->resolve_pathname(timepoint);
		SQLite3* dbc = new SQLite3(dbsource->Data(), this->get_logger());
		std::list<EarthWork> ews = select_earthwork(dbc, 0, 0, earthwork::timestamp, asc);
		double now = current_inexact_milliseconds();
		
		for (auto it = ews.begin(); it != ews.end(); it++) {
			long long ms = (*it).timestamp;

			this->get_logger()->log_message(Log::Info, L"%s.%03d",
				make_daytimestamp_utc(ms / 1000, true)->Data(), ms % 1000);
		}

		this->get_logger()->log_message(Log::Notice, L"loaded %d records from %s within %dms",
			ews.size(), dbsource->Data(), current_inexact_milliseconds() - now);
	} while (0);
}

void EarthWorkDataSource::save(long long timepoint, double* values, unsigned int n) {
	EarthWork ework;

	ework.uuid = pk64_timestamp();
	ework.timestamp = timepoint;

	for (unsigned int i = 0; i < n; i++) {
		switch (_E(EWTS, i)) {
		case EWTS::EarthWork: ework.product = values[i]; break;
		case EWTS::Vessel : ework.vessel = values[i]; break;
		case EWTS::HopperHeight: ework.hopper_height = values[i]; break;
		case EWTS::Loading: ework.loading = values[i]; break;
		case EWTS::Displacement: ework.displacement = values[i]; break;
		}
	}

	insert_earthwork(this, ework);
}
