#pragma once

#include "syslog.hpp"

namespace WarGrey::SCADA {
	private class IPLCClient abstract {
	public:
		virtual Platform::String^ device_hostname() = 0;
		virtual Syslog* get_logger() = 0;
		virtual bool connected() = 0;

	public:
		virtual void send_scheduled_request(long long count, long long interval, long long uptime, bool is_slow) = 0;
	};
}
