/*
 * Core/Shell/Scripts/TimeScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_TIMESCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_TIMESCRIPT_HPP_

#include "Shell/ShellScript.hpp"

class TimeScript: public ShellScript
{
public:
  TimeScript(Script *, ArgumentIterator, ArgumentIterator);
  virtual Result run() override;

  static const char *name()
  {
    return "time";
  }
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_TIMESCRIPT_HPP_
