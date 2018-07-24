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
  EchoScript(Script *, ArgumentIterator, ArgumentIterator);
  virtual Result run() override;

  static const char *name()
  {
    return "echo";
  }
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_ECHOSCRIPT_HPP_
