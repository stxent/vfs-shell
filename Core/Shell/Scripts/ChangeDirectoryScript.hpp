/*
 * Core/Shell/Scripts/ChangeDirectoryScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_CHANGEDIRECTORYSCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_CHANGEDIRECTORYSCRIPT_HPP_

#include <iterator>
#include "Shell/ArgParser.hpp"
#include "Shell/Settings.hpp"
#include "Shell/ShellScript.hpp"

class ChangeDirectoryScript: public ShellScript
{
public:
  ChangeDirectoryScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    ShellScript{parent, firstArgument, lastArgument}
  {
  }

  virtual Result run() override
  {
    static const ArgParser::Descriptor descriptors[] = {
        {"--help", nullptr, "show this help message and exit", 0, Arguments::helpSetter},
        {nullptr, "ENTRY", "change current directory to ENTRY", 1, Arguments::pathSetter}
    };

    bool argumentsParsed;
    const Arguments arguments = ArgParser::parse<Arguments>(m_firstArgument, m_lastArgument,
        std::cbegin(descriptors), std::cend(descriptors), &argumentsParsed);

    if (arguments.help)
    {
      ArgParser::help(tty(), name(), std::cbegin(descriptors), std::cend(descriptors));
      return E_OK;
    }
    else if (argumentsParsed)
    {
      return changeDirectory(arguments.path);
    }
    else
    {
      return E_VALUE;
    }
  }

  static const char *name()
  {
    return "cd";
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

  Result changeDirectory(const char *relativePath)
  {
    char path[Settings::PWD_LENGTH];

    ShellHelpers::joinPaths(path, env()["PWD"], relativePath);
    FsNode * const root = ShellHelpers::openNode(fs(), path);
    if (root == nullptr)
    {
      tty() << name() << ": " << relativePath << ": no such node" << Terminal::EOL;
      return E_ENTRY;
    }

    // Check node type
    FsNode * const child = static_cast<FsNode *>(fsNodeHead(root));
    fsNodeFree(root);
    if (child == nullptr)
    {
      tty() << name() << ": " << relativePath << ": not a directory" << Terminal::EOL;
      return E_ENTRY;
    }

    FsAccess access;
    const Result res = fsNodeRead(child, FS_NODE_ACCESS, 0, &access, sizeof(access), nullptr);
    fsNodeFree(child);

    if (res != E_OK)
    {
      tty() << name() << ": " << relativePath << ": error reading access attribute" << Terminal::EOL;
      return res;
    }

    // TODO Hierarchical check
    if (!(access & FS_ACCESS_READ))
    {
      tty() << name() << ": " << relativePath << ": permission denied" << Terminal::EOL;
      return E_ACCESS;
    }

    env()["PWD"] = path;
    return E_OK;
  }
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_CHANGEDIRECTORYSCRIPT_HPP_
