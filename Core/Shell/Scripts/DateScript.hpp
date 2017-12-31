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
    RtDateTime currentTime;
    rtMakeTime(&currentTime, time().microtime() / 1000000);

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

  static const char *name()
  {
    return "date";
  }
};

#endif // VFS_SHELL_SHELL_SCRIPTS_DATESCRIPT_HPP_
