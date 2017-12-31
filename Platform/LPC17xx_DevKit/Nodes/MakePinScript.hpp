/*
 * Platform/LPC17xx_DevKit/Nodes/MakePinScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_NODES_MAKEPINSCRIPT_HPP_
#define VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_NODES_MAKEPINSCRIPT_HPP_

#include "Nodes/PinNode.hpp"
#include "Shell/Settings.hpp"
#include "Shell/ShellHelpers.hpp"
#include "Shell/ShellScript.hpp"

class MakePinScript: public ShellScript
{
public:
  MakePinScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    ShellScript{parent, firstArgument, lastArgument}
  {
  }

  virtual Result run() override
  {
    Arguments args;
    const std::array<ArgParser::Descriptor, 2> argTable = {
        {
            {"--help", "print help message", 0, boolArgumentSetter, &args.showHelpMessage},
            {nullptr, nullptr, 2, positionalArgumentParser, &args}
        }
    };

    ArgParser::parse(m_firstArgument, m_lastArgument, argTable.begin(), argTable.end());

    if (args.name == nullptr || args.port == -1 || args.offset == -1 || args.showHelpMessage)
    {
      ArgParser::help(tty(), argTable.begin(), argTable.end());
      return E_OK;
    }
    else
    {
      char path[Settings::PWD_LENGTH];
      ShellHelpers::joinPaths(path, env()["PWD"], args.name);

      FsNode * const root = ShellHelpers::openBaseNode(fs(), path);
      if (root == nullptr)
      {
        tty() << name() << ": " << path << ": parent directory not found" << Terminal::EOL;
        return E_ENTRY;
      }

      // Create VFS node
      VfsNode * const entry = new PinNode{ShellHelpers::extractName(args.name), args.port,
          args.offset, time().microtime(), FS_ACCESS_READ | FS_ACCESS_WRITE};

      // Link VFS node to the existing file tree
      const FsFieldDescriptor descriptors[] = {
          {&entry, sizeof(entry), static_cast<FsFieldType>(VfsNode::VFS_NODE_OBJECT)},
      };
      const Result res = fsNodeCreate(root, descriptors, ARRAY_SIZE(descriptors));

      fsNodeFree(root);

      return res;
    }
  }

  static const char *name()
  {
    return "mkpin";
  }

private:
  struct Arguments
  {
    Arguments() :
      name{nullptr},
      port{-1},
      offset{-1},
      showHelpMessage{false}
    {
    }

    const char *name;
    long port;
    long offset;
    bool showHelpMessage;
  };

  static void boolArgumentSetter(void *object, const char *)
  {
    *static_cast<bool *>(object) = true;
  }

  static void positionalArgumentParser(void *object, const char *argument)
  {
    auto * const args = static_cast<Arguments *>(object);

    if (args->name == nullptr)
      args->name = argument;
    else if (args->port == -1)
      args->port = atol(argument);
    else if (args->offset == -1)
      args->offset = atol(argument);
    else
      args->showHelpMessage = true; // Incorrect argument count
  }
};

#endif // VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_NODES_MAKEPINSCRIPT_HPP_
