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
  DateScript(Script *, ArgumentIterator, ArgumentIterator);
  virtual Result run() override;

  static const char *name()
  {
    return "date";
  }

private:
  struct Arguments
  {
    const char *alarm{nullptr};
    const char *time{nullptr};
    bool help{false};

    static void alarmSetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->alarm = argument;
    }

    static void timeSetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->time = argument;
    }

    static void helpSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->help = true;
    }
  };

  Result setAlarm(const char *);
  Result setTime(const char *);
  Result showTime();

  static bool stringToTimestamp(const char *, time64_t *);
};

#endif // VFS_SHELL_SHELL_SCRIPTS_DATESCRIPT_HPP_
