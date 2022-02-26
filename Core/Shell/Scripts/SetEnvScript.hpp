/*
 * Core/Shell/Scripts/SetEnvScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_SETENVSCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_SETENVSCRIPT_HPP_

#include "Shell/ShellScript.hpp"

class SetEnvScript: public ShellScript
{
public:
  SetEnvScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    ShellScript{parent, firstArgument, lastArgument}
  {
  }

  virtual Result run() override
  {
    if (m_firstArgument != m_lastArgument && m_firstArgument + 1 != m_lastArgument)
    {
      auto &variable = env()[*m_firstArgument];
      variable = *(m_firstArgument + 1);
    }

    return E_OK;
  }

  static const char *name()
  {
    return "setenv";
  }
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_SETENVSCRIPT_HPP_
