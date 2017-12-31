/*
 * Platform/LPC17xx_DevKit/RealTimeClock.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_ENTRIES_REALTIMECLOCK_HPP_
#define VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_ENTRIES_REALTIMECLOCK_HPP_

#include <functional>
#include <limits>
#include <memory>
#include <halm/platform/nxp/gptimer.h>
#include <halm/platform/nxp/rtc.h>
#include "Shell/TimeProvider.hpp"

class RealTimeClock: public TimeProvider
{
public:
  RealTimeClock(const RealTimeClock &) = delete;
  RealTimeClock &operator=(const RealTimeClock &) = delete;

  virtual time64_t microtime() override
  {
    time64_t globalTick;
    uint32_t currentTick;

    do {
      globalTick = m_tick;
      currentTick = timerGetValue(m_monotonic.get());
    } while (globalTick != m_tick);

    return globalTick + static_cast<time64_t>(currentTick);
  }

  static RealTimeClock &instance()
  {
    static RealTimeClock object;
    return object;
  }

private:
  static constexpr time64_t PERIOD = static_cast<time64_t>(std::numeric_limits<uint32_t>::max());

  RealTimeClock() :
    m_monotonic{static_cast<Timer *>(init(GpTimer, &s_monotonicTimerConfig)), [](Timer *pointer){ deinit(pointer); }},
    m_realtime{static_cast<RtClock *>(init(Rtc, &s_realtimeClockConfig)), [](RtClock *pointer){ deinit(pointer); }},
    m_tick{rtTime(m_realtime.get()) * 1000000}
  {
    timerEnable(m_monotonic.get());
  }

  static void onTimerOverflow(void *argument)
  {
    static_cast<RealTimeClock *>(argument)->m_tick += PERIOD;
  }

  std::unique_ptr<Timer, std::function<void (Timer *)>> m_monotonic;
  std::unique_ptr<RtClock, std::function<void (RtClock *)>> m_realtime;
  time64_t m_tick;

  static const GpTimerConfig s_monotonicTimerConfig;
  static const RtcConfig s_realtimeClockConfig;
};

#endif // VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_ENTRIES_REALTIMECLOCK_HPP_
