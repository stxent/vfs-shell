/*
 * Core/Shell/Scripts/DateScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_DATESCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_DATESCRIPT_HPP_

#include "Shell/ShellScript.hpp"
#include <xcore/realtime.h>

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
    const char *limit{nullptr};
    const char *time{nullptr};
    bool alarm{false};
    bool help{false};

    static void alarmSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->alarm = true;
    }

    static void helpSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->help = true;
    }

    static void limitSetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->limit = argument;
    }

    static void timeSetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->time = argument;
    }
  };

  void printTime(time64_t);
  Result setAlarm(const char *);
  Result setTime(const char *);
  Result showAlarm();
  Result showTime();

  static bool stringToTimestamp(const char *, time64_t *);
};

#endif // VFS_SHELL_SHELL_SCRIPTS_DATESCRIPT_HPP_
