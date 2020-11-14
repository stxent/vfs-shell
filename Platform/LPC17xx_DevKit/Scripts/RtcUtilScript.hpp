/*
 * Platform/LPC17xx_DevKit/Scripts/RtcUtilScript.hpp
 * Copyright (C) 2019 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_SCRIPTS_RTCUTILSCRIPT_HPP_
#define VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_SCRIPTS_RTCUTILSCRIPT_HPP_

#include "Shell/ShellScript.hpp"
#include <halm/core/cortex/nvic.h>
#include <halm/platform/nxp/backup_domain.h>
#include <halm/pm.h>
#include <atomic>

template<typename T>
class RtcUtilScript: public ShellScript
{
public:
  RtcUtilScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument, T *clock) :
    ShellScript{parent, firstArgument, lastArgument},
    m_clock{clock},
    m_flag{false}
  {
  }

  virtual Result run() override
  {
    static const ArgParser::Descriptor descriptors[] = {
        {"--help", nullptr, "show this help message and exit", 0, Arguments::helpSetter},
        {"-s", nullptr, "show clock skew", 0, Arguments::skewSetter}
    };

    const Arguments arguments = ArgParser::parse<Arguments>(m_firstArgument, m_lastArgument,
        std::cbegin(descriptors), std::cend(descriptors));

    if (arguments.help || !arguments.skew)
    {
      ArgParser::help(tty(), name(), std::cbegin(descriptors), std::cend(descriptors));
      return E_OK;
    }
    else if (arguments.skew)
    {
      const auto skew = measureClockSkew();
      const auto fill = tty().fill();
      const auto width = tty().width();

      if (skew < 0)
        tty() << '-';
      tty() << (abs(skew) / 1000000) << '.';
      tty() << Terminal::Width{6} << Terminal::Fill{'0'} << (abs(skew) % 1000000) << fill << width;
      tty() << " s" << Terminal::EOL;

      return E_OK;
    }
    else
    {
      return E_VALUE;
    }
  }

  static const char *name()
  {
    return "rtc-util";
  }

private:
  struct Arguments
  {
    bool help{false};
    bool skew{false};

    static void helpSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->help = true;
    }

    static void skewSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->skew = true;
    }
  };

  T *m_clock;
  std::atomic<bool> m_flag;

  int measureClockSkew()
  {
    int result = 0;

    m_clock->setCallback([this](){ m_flag = true; });

    do
    {
      const auto rawTime = m_clock->getRawTime();
      const auto alarmTime = rawTime + 1000000;
      const auto retryTime = rawTime + 2000000;

      m_flag = false;
      m_clock->setAlarm(alarmTime);

      while (m_clock->getRawTime() < retryTime)
      {
        if (m_flag)
        {
          result = static_cast<int>(m_clock->getTime() - m_clock->getRawTime());
          break;
        }
      }
    }
    while (!m_flag);

    m_clock->setCallback(nullptr);
    return result;
  }
};

#endif // VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_SCRIPTS_RTCUTILSCRIPT_HPP_
