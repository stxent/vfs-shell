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
    static const ArgParser::Descriptor descriptors[] = {
        {"--help", nullptr, "show this help message and exit", 0, Arguments::helpSetter},
        {"-o", "VALUE", "configure pin as output and write VALUE", 1, Arguments::outputSetter},
        {nullptr, "PORT", "input/output port number", 1, Arguments::portSetter},
        {nullptr, "PIN", "input/output pin number", 1, Arguments::pinSetter},
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
    else if (!argumentsParsed)
    {
      tty() << name() << ": incorrect arguments" << Terminal::EOL;
      return E_VALUE;
    }
    else
    {
      return makePin(arguments.path, arguments.port, arguments.pin, arguments.output, arguments.value);
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
      path{nullptr},
      pin{-1},
      port{-1},
      help{false},
      output{false},
      value{false}
    {
    }

    const char *path;
    long pin;
    long port;
    bool help;
    bool output;
    bool value;

    static void pathSetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->path = argument;
    }

    static void pinSetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->pin = static_cast<size_t>(atol(argument));
    }

    static void portSetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->port = static_cast<size_t>(atol(argument));
    }

    static void helpSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->help = true;
    }

    static void outputSetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->output = true;
      static_cast<Arguments *>(object)->value = atol(argument) != 0;
    }
  };

  Result makePin(const char *path, long port, long pin, bool output, bool value)
  {
    char absolutePath[Settings::PWD_LENGTH];
    ShellHelpers::joinPaths(absolutePath, env()["PWD"], path);

    // Check node existence
    FsNode * const existingNode = ShellHelpers::openNode(fs(), absolutePath);
    if (existingNode != nullptr)
    {
      fsNodeFree(existingNode);
      return E_EXIST;
    }

    FsNode * const root = ShellHelpers::openBaseNode(fs(), absolutePath);

    if (root != nullptr)
    {
      // Create VFS node
      VfsNode *entry;

      // Initialize pin as output or input depending on the arguments of the command
      if (output)
        entry = new PinNode{ShellHelpers::extractName(path), port, pin, value, time().get()};
      else
        entry = new PinNode{ShellHelpers::extractName(path), port, pin, time().get()};

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

#endif // VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_NODES_MAKEPINSCRIPT_HPP_
