/*
 * UnixTimeProvider.cpp
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <cstdlib>
#include <ctime>
#include <halm/platform/linux/rtc.h>
#include "UnixTimeProvider.hpp"

UnixTimeProvider::~UnixTimeProvider()
{
  deinit(clock);
}

time64_t UnixTimeProvider::get()
{
  struct timespec currentTime;

  if (!clock_gettime(CLOCK_REALTIME, &currentTime))
    return currentTime.tv_sec * 1000000 + currentTime.tv_nsec / 1000;
  else
    return 0;
}

UnixTimeProvider::UnixTimeProvider()
{
  // TODO unique_ptr
  clock = static_cast<RtClock *>(init(Rtc, nullptr));

  if (clock == nullptr)
    exit(EXIT_FAILURE);
}
