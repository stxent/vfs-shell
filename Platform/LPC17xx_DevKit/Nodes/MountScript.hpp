/*
 * Platform/LPC17xx_DevKit/Nodes/MountScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_NODES_MOUNTSCRIPT_HPP_
#define VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_NODES_MOUNTSCRIPT_HPP_

#include "Shell/Scripts/MountScriptBase.hpp"

template<class T>
class MountScript: public MountScriptBase
{
public:
  MountScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument, Interface *interface) :
    MountScriptBase{parent, firstArgument, lastArgument},
    m_interface{interface}
  {
  }

  virtual Result run() override
  {
    static const ArgParser::Descriptor descriptors[] = {
        {"--help", nullptr, "show this help message and exit", 0, Arguments::helpSetter},
        {nullptr, "ENTRY", "path to the virtual entry", 1, Arguments::entrySetter}
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
      return E_VALUE;
    }

    Interface * const interface = T::build(m_interface);
    if (interface == nullptr)
    {
      tty() << name() << ": file not found" << Terminal::EOL;
      return E_VALUE;
    }

    const Result res = mount(arguments.entry, interface);
    if (res != E_OK)
      deinit(interface);

    return res;
  }

private:
  Interface * const m_interface;

  struct Arguments
  {
    Arguments() :
      entry{nullptr},
      help{false}
    {
    }

    const char *entry;
    bool help;

    static void helpSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->help = true;
    }

    static void entrySetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->entry = argument;
    }
  };
};

#endif // VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_NODES_MOUNTSCRIPT_HPP_
