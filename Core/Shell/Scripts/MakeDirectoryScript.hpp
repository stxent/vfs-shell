/*
 * Core/Shell/Scripts/MakeDirectoryScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_MAKEDIRECTORYSCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_MAKEDIRECTORYSCRIPT_HPP_

#include "Shell/ShellScript.hpp"

class MakeDirectoryScript: public ShellScript
{
public:
  MakeDirectoryScript(Script *, ArgumentIterator, ArgumentIterator);
  virtual Result run() override;

  static const char *name()
  {
    return "mkdir";
  }

private:
  struct Arguments
  {
    Arguments() :
      path{nullptr},
      help{false}
    {
    }

    const char *path;
    bool help;

    static void helpSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->help = true;
    }

    static void pathSetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->path = argument;
    }
  };

  Result makeDirectory(const char *);
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_MAKEDIRECTORYSCRIPT_HPP_
