/*
 * RealTimeClock.cpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "RealTimeClock.hpp"

const GpTimerConfig RealTimeClock::s_monotonicTimerConfig = {
    1000000,            // frequency
    GPTIMER_MATCH_AUTO, // event
    0,                  // priority
    3                   // channel
};

const RtcConfig RealTimeClock::s_realtimeClockConfig = {
    0, // timestamp
    0  // priority
};
