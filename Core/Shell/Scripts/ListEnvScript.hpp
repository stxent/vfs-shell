/*
 * Core/Shell/Scripts/ListEnvScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_LISTENVSCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_LISTENVSCRIPT_HPP_

#include "Shell/ShellScript.hpp"

class ListEnvScript: public ShellScript
{
public:
  ListEnvScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    ShellScript{parent, firstArgument, lastArgument}
  {
  }

  virtual Result run() override
  {
    env().iterate([this](const char *key, const char *value){ printEnvInfo(key, value); });
    return E_OK;
  }

  static const char *name()
  {
    return "env";
  }

private:
  void printEnvInfo(const char *envName, const char *envValue)
  {
    tty() << envName << '=' << envValue << Terminal::EOL;
  }
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_LISTENVSCRIPT_HPP_
