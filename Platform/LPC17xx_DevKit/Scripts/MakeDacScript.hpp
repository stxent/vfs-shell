/*
 * Platform/LPC17xx_DevKit/Scripts/MakeDacScript.hpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_SCRIPTS_MAKEDACSCRIPT_HPP_
#define VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_SCRIPTS_MAKEDACSCRIPT_HPP_

#include "Nodes/DacNode.hpp"
#include "Shell/Settings.hpp"
#include "Shell/ShellHelpers.hpp"
#include "Shell/ShellScript.hpp"
#include <xcore/fs/utils.h>

class MakeDacScript: public ShellScript
{
public:
  MakeDacScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    ShellScript{parent, firstArgument, lastArgument}
  {
  }

  virtual Result run() override
  {
    static const ArgParser::Descriptor descriptors[] = {
        {"--help", nullptr, "show this help message and exit", 0, Arguments::helpSetter},
        {nullptr, "ENTRY", "file system entry", 1, Arguments::pathSetter}
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
      return makeDac(arguments.path);
    }
    else
    {
      return E_VALUE;
    }
  }

  static const char *name()
  {
    return "mkdac";
  }

private:
  struct Arguments
  {
    const char *path{nullptr};
    bool help{false};

    static void pathSetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->path = argument;
    }

    static void helpSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->help = true;
    }
  };

  Result makeDac(const char *path)
  {
    char absolutePath[Settings::PWD_LENGTH];
    fsJoinPaths(absolutePath, env()["PWD"], path);

    // Check node existence
    FsNode * const existingNode = fsOpenNode(fs(), absolutePath);
    if (existingNode != nullptr)
    {
      fsNodeFree(existingNode);
      return E_EXIST;
    }

    FsNode * const root = fsOpenBaseNode(fs(), absolutePath);

    if (root != nullptr)
    {
      // Create VFS node
      VfsNode * const entry = new DacNode{fsExtractName(path), time().getTime()};

      // Link VFS node to the existing file tree
      const FsFieldDescriptor fields[] = {
          {&entry, sizeof(entry), static_cast<FsFieldType>(VfsNode::VFS_NODE_OBJECT)},
      };
      const Result res = fsNodeCreate(root, fields, ARRAY_SIZE(fields));

      fsNodeFree(root);
      return res;
    }
    else
    {
      tty() << name() << ": " << absolutePath << ": parent directory not found" << Terminal::EOL;
      return E_ENTRY;
    }
  }
};

#endif // VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_SCRIPTS_MAKEDACSCRIPT_HPP_
