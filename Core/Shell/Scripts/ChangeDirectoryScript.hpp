/*
 * Core/Shell/Scripts/ChangeDirectoryScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_CHANGEDIRECTORYSCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_CHANGEDIRECTORYSCRIPT_HPP_

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
    Arguments arguments;
    const std::array<ArgParser::Descriptor, 2> descriptors{
        {
            {"--help", "print help message", 0, boolArgumentSetter, &arguments.showHelpMessage},
            {nullptr, nullptr, 1, pathExtractor, &arguments}
        }
    };

    ArgParser::parse(m_firstArgument, m_lastArgument, descriptors.begin(), descriptors.end());

    if (arguments.showHelpMessage || arguments.relativePath == nullptr)
    {
      ArgParser::help(tty(), descriptors.begin(), descriptors.end());
      return E_OK;
    }
    else
      return changeDirectory(arguments.relativePath);
  }

  static const char *name()
  {
    return "cd";
  }

private:
  struct Arguments
  {
    Arguments() :
        relativePath{nullptr},
        showHelpMessage{false}
    {
    }

    const char *relativePath;
    bool showHelpMessage;
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

  static void boolArgumentSetter(void *object, const char *)
  {
    *static_cast<bool *>(object) = true;
  }

  static void pathExtractor(void *object, const char *argument)
  {
    auto * const args = static_cast<Arguments *>(object);

    if (args->relativePath == nullptr)
      args->relativePath = argument;
    else
      args->showHelpMessage = true;
  }
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_CHANGEDIRECTORYSCRIPT_HPP_
