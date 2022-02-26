/*
 * Platform/LPC17xx_DevKit/Scripts/MakePinScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_SCRIPTS_MAKEPINSCRIPT_HPP_
#define VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_SCRIPTS_MAKEPINSCRIPT_HPP_

#include "Nodes/PinNode.hpp"
#include "Shell/Settings.hpp"
#include "Shell/ShellHelpers.hpp"
#include "Shell/ShellScript.hpp"
#include <xcore/fs/utils.h>

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
    else if (argumentsParsed)
    {
      return makePin(arguments.path, arguments.port, arguments.pin, arguments.output, arguments.value);
    }
    else
    {
      return E_VALUE;
    }
  }

  static const char *name()
  {
    return "mkpin";
  }

private:
  struct Arguments
  {
    const char *path{nullptr};
    long pin{-1};
    long port{-1};
    bool help{false};
    bool output{false};
    bool value{false};

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
    fsJoinPaths(absolutePath, env()["PWD"], path);

    // Check node existence
    FsNode * const existingNode = fsOpenNode(fs(), absolutePath);
    if (existingNode != nullptr)
    {
      fsNodeFree(existingNode);
      return E_EXIST;
    }

    // Create VFS node
    VfsNode *entry;
    Result res;

    // Initialize pin as output or input depending on the arguments of the command
    if (output)
      entry = new PinNode{port, pin, value, time().getTime()};
    else
      entry = new PinNode{port, pin, time().getTime()};

    if (entry != nullptr)
    {
      res = ShellHelpers::injectNode(fs(), entry, absolutePath);

      if (res != E_OK)
      {
        tty() << name() << ": node linking failed" << Terminal::EOL;
        delete entry;
      }
    }
    else
    {
      tty() << name() << ": node creation failed" << Terminal::EOL;
      res = E_MEMORY;
    }

    return res;
  }
};

#endif // VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_SCRIPTS_MAKEPINSCRIPT_HPP_
