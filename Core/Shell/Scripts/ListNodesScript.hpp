/*
 * Core/Shell/Scripts/ListNodesScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_LISTNODESSCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_LISTNODESSCRIPT_HPP_

#include <array>
#include "Shell/ArgParser.hpp"
#include "Shell/ShellScript.hpp"

class ListNodesScript: public ShellScript
{
public:
  ListNodesScript(Script *, ArgumentIterator, ArgumentIterator);
  virtual Result run() override;

  static const char *name()
  {
    return "ls";
  }

private:
  struct HumanReadableAccess
  {
    FsAccess value;
    bool directory;
  };

  struct HumanReadableLength
  {
    FsLength value;
  };

  struct HumanReadableTime
  {
    time64_t value;
  };

  friend Terminal &operator<<(Terminal &, HumanReadableAccess);
  friend Terminal &operator<<(Terminal &, HumanReadableLength);
  friend Terminal &operator<<(Terminal &, HumanReadableTime);

  struct Arguments
  {
    Arguments() :
      nodeCount{0},
      help{false},
      humanReadable{false},
      longListing{false},
      showInodes{false}
    {
    }

    size_t nodeCount;
    bool help;
    bool humanReadable;
    bool longListing;
    bool showInodes;

    static void incrementNodeCount(void *object, const char *)
    {
      ++static_cast<Arguments *>(object)->nodeCount;
    }

    static void helpSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->help = true;
    }

    static void humanReadableSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->humanReadable = true;
    }

    static void longListingSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->longListing = true;
    }

    static void showInodesSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->showInodes = true;
    }
  };

  const Arguments m_arguments;
  Result m_result;

  void printDirectoryContent(const char *);

  static const std::array<ArgParser::Descriptor, 5> descriptors;
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_LISTNODESSCRIPT_HPP_
