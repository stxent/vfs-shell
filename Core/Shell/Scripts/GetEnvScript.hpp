/*
 * Core/Shell/Scripts/GetEnvScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_GETENVSCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_GETENVSCRIPT_HPP_

#include "Shell/ShellScript.hpp"

class GetEnvScript: public ShellScript
{
public:
  GetEnvScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    ShellScript{parent, firstArgument, lastArgument}
  {
  }

  virtual Result run() override
  {
    if (m_firstArgument != m_lastArgument)
      tty() << env()[*m_firstArgument] << Terminal::EOL;

    return E_OK;
  }

  static const char *name()
  {
    return "getenv";
  }
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_GETENVSCRIPT_HPP_
