/*
 * UnixTimeProvider.cpp
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "UnixTimeProvider.hpp"
#include <ctime>

time64_t UnixTimeProvider::getTime()
{
  struct timespec currentTime;

  if (!clock_gettime(CLOCK_REALTIME, &currentTime))
    return currentTime.tv_sec * 1000000 + currentTime.tv_nsec / 1000;
  else
    return 0;
}
