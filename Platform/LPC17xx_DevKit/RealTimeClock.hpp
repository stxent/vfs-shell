/*
 * Platform/LPC17xx_DevKit/RealTimeClock.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_ENTRIES_REALTIMECLOCK_HPP_
#define VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_ENTRIES_REALTIMECLOCK_HPP_

#include "Shell/TimeProvider.hpp"
#include <halm/platform/lpc/gptimer.h>
#include <halm/platform/lpc/rtc.h>
#include <functional>
#include <limits>
#include <memory>

class RealTimeClock: public TimeProvider
{
public:
  RealTimeClock(const RealTimeClock &) = delete;
  RealTimeClock &operator=(const RealTimeClock &) = delete;

  virtual time64_t getTime() override
  {
    time64_t globalTick;
    uint32_t currentTick;

    do {
      globalTick = m_tick;
      currentTick = timerGetValue(m_monotonic.get());
    } while (globalTick != m_tick);

    return globalTick + static_cast<time64_t>(currentTick);
  }

  virtual Result setAlarm(time64_t timestamp) override
  {
    return rtSetAlarm(m_realtime.get(), timestamp / 1000000);
  }

  virtual Result setTime(time64_t timestamp) override
  {
    const RtcConfig updatedRealtimeClockConfig{timestamp / 1000000, 0};

    m_realtime.reset();
    m_realtime = {static_cast<RtClock *>(init(Rtc, &updatedRealtimeClockConfig)), clockDeleter};

    if (m_realtime != nullptr)
    {
      timerDisable(m_monotonic.get());
      timerSetValue(m_monotonic.get(), 0);
      m_tick = {rtTime(m_realtime.get()) * 1000000};
      timerEnable(m_monotonic.get());
      return E_OK;
    }
    else
      return E_VALUE;
  }

  time64_t getRawTime()
  {
    return rtTime(m_realtime.get()) * 1000000;
  }

  void setCallback(std::function<void ()> callback)
  {
    if (callback)
    {
      m_callback = callback;
      rtSetCallback(m_realtime.get(), onTimerAlarm, this);
    }
    else
    {
      rtSetCallback(m_realtime.get(), nullptr, nullptr);
      m_callback = nullptr;
    }
  }

  static RealTimeClock &instance()
  {
    static RealTimeClock object;
    return object;
  }

private:
  static constexpr auto PERIOD{static_cast<time64_t>(std::numeric_limits<uint32_t>::max())};

  static void clockDeleter(RtClock *pointer)
  {
    deinit(pointer);
  }

  RealTimeClock() :
    m_monotonic{static_cast<Timer *>(init(GpTimer, &s_monotonicTimerConfig)), [](Timer *pointer){ deinit(pointer); }},
    m_realtime{static_cast<RtClock *>(init(Rtc, &s_realtimeClockConfig)), clockDeleter},
    m_callback{},
    m_tick{rtTime(m_realtime.get()) * 1000000}
  {
    timerEnable(m_monotonic.get());
  }

  static void onTimerAlarm(void *argument)
  {
    static_cast<RealTimeClock *>(argument)->m_callback();
  }

  static void onTimerOverflow(void *argument)
  {
    static_cast<RealTimeClock *>(argument)->m_tick += PERIOD;
  }

  std::unique_ptr<Timer, std::function<void (Timer *)>> m_monotonic;
  std::unique_ptr<RtClock, std::function<void (RtClock *)>> m_realtime;
  std::function<void ()> m_callback;
  time64_t m_tick;

  static const GpTimerConfig s_monotonicTimerConfig;
  static const RtcConfig s_realtimeClockConfig;
};

#endif // VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_ENTRIES_REALTIMECLOCK_HPP_
