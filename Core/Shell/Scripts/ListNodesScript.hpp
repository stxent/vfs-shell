/*
 * Core/Shell/Scripts/ListNodesScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_LISTNODESSCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_LISTNODESSCRIPT_HPP_

#include "Shell/ShellScript.hpp"

class ListNodesScript: public ShellScript
{
public:
  ListNodesScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    ShellScript{parent, firstArgument, lastArgument},
    m_result{E_OK}
  {
  }

  virtual Result run() override;

  static const char *name()
  {
    return "ls";
  }

private:
  struct Arguments
  {
    Arguments() :
      nodeCount{0},
      humanReadable{false},
      longListing{false},
      showInodes{false},
      showHelpMessage{false}
    {
    }

    size_t nodeCount;
    bool humanReadable;
    bool longListing;
    bool showInodes;
    bool showHelpMessage;
  };

  Arguments m_arguments;
  Result m_result;

  void printDirectoryContent(const char *);

  static void boolArgumentSetter(void *object, const char *)
  {
    *static_cast<bool *>(object) = true;
  }

  static void incrementNodeCounter(void *object, const char *)
  {
    *static_cast<size_t *>(object) += 1;
  }

  struct HumanReadableLength
  {
    FsLength value;
  };

  friend Terminal &operator<<(Terminal &, HumanReadableLength);
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_LISTNODESSCRIPT_HPP_
