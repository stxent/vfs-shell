/*
 * Core/Shell/Scripts/RemoveNodesScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_REMOVENODESSCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_REMOVENODESSCRIPT_HPP_

#include "Shell/ShellScript.hpp"

class RemoveNodesScript: public ShellScript
{
public:
  RemoveNodesScript(Script *, ArgumentIterator, ArgumentIterator);
  virtual Result run() override;

  static const char *name()
  {
    return "rm";
  }

private:
  struct Arguments
  {
    Arguments() :
      help{false},
      recursive{false}
    {
    }

    bool help;
    bool recursive;

    static void helpSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->help = true;
    }

    static void recursiveSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->recursive = true;
    }
  };

  Result m_result;

  void removeNode(const char *, bool);
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_REMOVENODESSCRIPT_HPP_
