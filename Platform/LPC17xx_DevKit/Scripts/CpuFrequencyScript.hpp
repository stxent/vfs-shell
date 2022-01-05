/*
 * Platform/LPC17xx_DevKit/Scripts/CpuFrequencyScript.hpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_SCRIPTS_CPUFREQUENCYSCRIPT_HPP_
#define VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_SCRIPTS_CPUFREQUENCYSCRIPT_HPP_

#include "Shell/ArgParser.hpp"
#include "Shell/ShellScript.hpp"
#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/system.h>
#include <halm/pm.h>
#include <limits>
#include <tuple>

template<unsigned long INPUT_FREQUENCY>
class CpuFrequencyScript: public ShellScript
{
  struct PllValues
  {
    unsigned int div{};
    unsigned int mul{};
  };

public:
  CpuFrequencyScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    ShellScript{parent, firstArgument, lastArgument}
  {
  }

  virtual Result run() override
  {
    static const ArgParser::Descriptor descriptors[] = {
        {"--help", nullptr, "show this help message and exit", 0, Arguments::helpSetter},
        {"-s", "FREQUENCY", "set CPU frequency to FREQUENCY", 1, Arguments::frequencySetter}
    };

    bool argumentsParsed;
    const Arguments arguments = ArgParser::parse<Arguments>(m_firstArgument, m_lastArgument,
        std::cbegin(descriptors), std::cend(descriptors), &argumentsParsed);

    if (arguments.help)
    {
      ArgParser::help(tty(), name(), std::cbegin(descriptors), std::cend(descriptors));
      return E_OK;
    }
    else if (!argumentsParsed)
    {
      return E_VALUE;
    }
    else if (arguments.frequency != 0)
    {
      const auto values = findPllConfig(arguments.frequency);

      if (values.mul != 0 && values.div != 0)
      {
        static const GenericClockConfig activeMainClockConfig = {CLOCK_PLL};
        static const GenericClockConfig temporaryMainClockConfig = {CLOCK_INTERNAL};
        const PllConfig updatedPllConfig = {
            CLOCK_EXTERNAL,                           // source
            static_cast<uint16_t>(values.div), // divisor
            static_cast<uint16_t>(values.mul)  // multiplier
        };

        clockEnable(MainClock, &temporaryMainClockConfig);
        while (!clockReady(MainClock));

        clockDisable(SystemPll);
        clockEnable(SystemPll, &updatedPllConfig);
        while (!clockReady(SystemPll));

        clockEnable(MainClock, &activeMainClockConfig);
        while (!clockReady(MainClock));

        // Update timer frequencies and baud rates of all interfaces
        pmChangeState(PM_ACTIVE);

        tty() << "multiplier: " << values.mul << Terminal::EOL;
        tty() << "divisor:    " << values.div << Terminal::EOL;

        return E_OK;
      }
      else
      {
        return E_VALUE;
      }
    }
    else
    {
      const auto mainClockFrequency = clockFrequency(MainClock);
      const auto flashLatency = sysFlashLatency();

      tty() << "frequency: " << mainClockFrequency << Terminal::EOL;
      tty() << "latency:   " << flashLatency << Terminal::EOL;

      return E_OK;
    }
  }

  static const char *name()
  {
    return "cpufreq";
  }

private:
  struct Arguments
  {
    unsigned long frequency{0};
    bool help{false};

    static void frequencySetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->frequency = static_cast<unsigned long>(atol(argument));
    }

    static void helpSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->help = true;
    }
  };

  static auto findPllConfig(unsigned long frequency)
  {
    // Divisor values: 1..32
    // Multiplier values: 6..512
    // CCO range: from 275 MHz to 550 MHz

    static constexpr unsigned long CCO_MIN{275000000UL};
    static constexpr unsigned long CCO_MAX{550000000UL};

    const unsigned int lowerMul = std::max(6UL, (CCO_MIN + INPUT_FREQUENCY - 1) / INPUT_FREQUENCY);
    const unsigned int upperMul = std::min(512UL, CCO_MAX / INPUT_FREQUENCY);

    unsigned long frequencyError = std::numeric_limits<unsigned long>::max();
    PllValues result{};

    for (auto m = lowerMul; m <= upperMul; ++m)
    {
      const auto ccoFrequency = m * INPUT_FREQUENCY;
      const unsigned int lowerDiv = std::max(1UL, ccoFrequency / frequency);
      const unsigned int upperDiv = std::min(32UL, (ccoFrequency + frequency - 1) / frequency);

      for (auto d = lowerDiv; d <= upperDiv; ++d)
      {
        const auto cpuFrequency = ccoFrequency / d;
        const auto currentError = static_cast<unsigned long>(std::labs(cpuFrequency - frequency));

        if (currentError < frequencyError)
        {
          frequencyError = currentError;
          result.div = d;
          result.mul = m;
        }
      }
    }

    return result;
  }
};

#endif // VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_SCRIPTS_CPUFREQUENCYSCRIPT_HPP_
