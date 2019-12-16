/*
 * Core/Shell/Scripts/ChangeDirectoryScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_CHANGEDIRECTORYSCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_CHANGEDIRECTORYSCRIPT_HPP_

#include "Shell/ShellScript.hpp"

class ChangeDirectoryScript: public ShellScript
{
public:
  ChangeDirectoryScript(Script *, ArgumentIterator, ArgumentIterator);
  virtual Result run() override;

  static const char *name()
  {
    return "cd";
  }

private:
  struct Arguments
  {
    const char *path{nullptr};
    bool help{false};

    static void helpSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->help = true;
    }

    static void pathSetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->path = argument;
    }
  };

  Result changeDirectory(const char *);
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_CHANGEDIRECTORYSCRIPT_HPP_
