/*
 * Core/Shell/Scripts/EchoScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_ECHOSCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_ECHOSCRIPT_HPP_

#include "Shell/ShellScript.hpp"

class EchoScript: public ShellScript
{
public:
  EchoScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    ShellScript{parent, firstArgument, lastArgument}
  {
  }

  virtual Result run() override
  {
    for (auto iter = m_firstArgument; iter != m_lastArgument; ++iter)
    {
      tty() << *iter;

      if (iter != m_lastArgument - 1)
        tty() << " ";
    }
    tty() << Terminal::EOL;

    return E_OK;
  }

  static const char *name()
  {
    return "echo";
  }
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_ECHOSCRIPT_HPP_
