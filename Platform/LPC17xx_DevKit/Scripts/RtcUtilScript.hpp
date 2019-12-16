/*
 * Platform/LPC17xx_DevKit/Scripts/RtcUtilScript.hpp
 * Copyright (C) 2019 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_SCRIPTS_RTCUTILSCRIPT_HPP_
#define VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_SCRIPTS_RTCUTILSCRIPT_HPP_

#include <halm/core/cortex/nvic.h>
#include <halm/platform/nxp/backup_domain.h>
#include <halm/pm.h>
#include "Shell/ShellScript.hpp"

template<typename T>
class RtcUtilScript: public ShellScript
{
public:
  RtcUtilScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument, T *clock) :
    ShellScript{parent, firstArgument, lastArgument},
    m_clock{clock}
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
      const auto skew = m_clock->getTime() - m_clock->getRawTime();

      tty() << static_cast<unsigned int>(skew) << Terminal::EOL;
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
};

#endif // VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_SCRIPTS_RTCUTILSCRIPT_HPP_
