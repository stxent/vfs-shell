/*
 * Core/Shell/Scripts/DateScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_DATESCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_DATESCRIPT_HPP_

#include <xcore/realtime.h>
#include "Shell/ShellScript.hpp"

class DateScript: public ShellScript
{
public:
  DateScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    ShellScript{parent, firstArgument, lastArgument}
  {
  }

  virtual Result run() override
  {
    static const ArgParser::Descriptor descriptors[] = {
        {"--help", nullptr, "show this help message and exit", 0, Arguments::helpSetter},
        {"-s", "STRING", "set time described by STRING", 1, Arguments::timeSetter}
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
      tty() << name() << ": incorrect arguments" << Terminal::EOL;
      return E_VALUE;
    }
    else if (arguments.time != nullptr)
    {
      return setTime(arguments.time);
    }
    else
    {
      return showTime();
    }
  }

  static const char *name()
  {
    return "date";
  }

private:
  struct Arguments
  {
    Arguments() :
      time{nullptr},
      help{false}
    {
    }

    const char *time;
    bool help;

    static void timeSetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->time = argument;
    }

    static void helpSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->help = true;
    }
  };

  Result setTime(const char *value)
  {
    unsigned int hour, minute, second, day, month, year;

    if (sscanf(value, "%u:%u:%u %u.%u.%u", &hour, &minute, &second, &day, &month, &year) == 6)
    {
      const RtDateTime dateTime = {
          static_cast<uint16_t>(year),
          static_cast<uint8_t>(month),
          static_cast<uint8_t>(day),
          static_cast<uint8_t>(hour),
          static_cast<uint8_t>(minute),
          static_cast<uint8_t>(second)
      };
      time64_t timestamp;

      Result res = rtMakeEpochTime(&timestamp, &dateTime);
      if (res == E_OK)
        res = time().set(timestamp * 1000000);

      return res;
    }
    else
    {
      return E_VALUE;
    }
  }

  Result showTime()
  {
    RtDateTime currentTime;
    rtMakeTime(&currentTime, time().get() / 1000000);

    // TODO Output types, restore initial terminal settings
    tty() << Terminal::Width{2} << Terminal::Fill{'0'};
    tty() << static_cast<unsigned int>(currentTime.hour);
    tty() << ":" << static_cast<unsigned int>(currentTime.minute);
    tty() << ":" << static_cast<unsigned int>(currentTime.second);
    tty() << " " << static_cast<unsigned int>(currentTime.day);
    tty() << "." << static_cast<unsigned int>(currentTime.month);
    tty() << Terminal::Width{4};
    tty() << "." << static_cast<unsigned int>(currentTime.year);
    tty() << Terminal::Fill{' '} << Terminal::Width{1};
    tty() << Terminal::EOL;

    return E_OK;
  }
};

#endif // VFS_SHELL_SHELL_SCRIPTS_DATESCRIPT_HPP_
