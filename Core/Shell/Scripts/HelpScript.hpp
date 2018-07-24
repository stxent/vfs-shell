/*
 * Core/Shell/Scripts/HelpScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_HELPSCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_HELPSCRIPT_HPP_

#include "Shell/ShellScript.hpp"

class HelpScript: public ShellScript
{
public:
  HelpScript(Script *, ArgumentIterator, ArgumentIterator);
  virtual Result run() override;

  static const char *name()
  {
    return "help";
  }
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_HELPSCRIPT_HPP_
