#pragma once

#include <cmath>
#include "snip/snip.hpp"

namespace WarGrey::SCADA {
    private class IPipeSnip : public WarGrey::SCADA::Snip {
    public:
        virtual Windows::Foundation::Rect get_input_port() = 0;
        virtual Windows::Foundation::Rect get_output_port() = 0;
    };

    void pipe_connecting_position(
        WarGrey::SCADA::IPipeSnip* prev,
        WarGrey::SCADA::IPipeSnip* pipe,
        float* x,
        float* y);

    Windows::Foundation::Numerics::float2 pipe_connecting_position(
        WarGrey::SCADA::IPipeSnip* prev,
        WarGrey::SCADA::IPipeSnip* pipe,
        float x = 0.0F,
        float y = 0.0F);
}
