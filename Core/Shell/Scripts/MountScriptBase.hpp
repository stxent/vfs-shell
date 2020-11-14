/*
 * Core/Shell/Scripts/MountScriptBase.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_MOUNTSCRIPTBASE_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_MOUNTSCRIPTBASE_HPP_

#include "Shell/ShellScript.hpp"
#include <xcore/interface.h>

class MountScriptBase: public ShellScript
{
public:
  MountScriptBase(Script *, ArgumentIterator, ArgumentIterator);

  static const char *name()
  {
    return "mount";
  }

protected:
  Result mount(const char *, Interface *);
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_MOUNTSCRIPTBASE_HPP_
