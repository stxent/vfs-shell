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
  static const auto &descriptors()
  {
    static const std::array<ArgParser::Descriptor, 5> descriptorArray = {
        {
          {"--help", nullptr, "show this help message and exit", 0, Arguments::helpSetter},
          {"-i", nullptr, "show index of each node", 0, Arguments::showInodesSetter},
          {"-h", nullptr, "print human readable sizes", 0, Arguments::humanReadableSetter},
          {"-l", nullptr, "show detailed information", 0, Arguments::longListingSetter},
          {nullptr, "ENTRY", "directory to be shown", 0, Arguments::incrementNodeCount}
        }
    };

    return descriptorArray;
  }

public:
  ListNodesScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    ShellScript{parent, firstArgument, lastArgument},
    m_arguments{ArgParser::parse<Arguments>(m_firstArgument, m_lastArgument,
        descriptors().cbegin(), descriptors().cend())},
    m_result{E_OK}
  {
  }

  virtual Result run() override;

  static const char *name()
  {
    return "ls";
  }

private:
  struct HumanReadableLength
  {
    FsLength value;
  };

  friend Terminal &operator<<(Terminal &, HumanReadableLength);

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
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_LISTNODESSCRIPT_HPP_
