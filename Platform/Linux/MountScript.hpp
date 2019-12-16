/*
 * Core/Shell/Scripts/MountScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_PLATFORM_LINUX_MOUNTSCRIPT_HPP_
#define VFS_SHELL_PLATFORM_LINUX_MOUNTSCRIPT_HPP_

#include <iterator>
#include "Shell/Scripts/MountScriptBase.hpp"

template<typename T>
class MountScript: public MountScriptBase
{
public:
  MountScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    MountScriptBase{parent, firstArgument, lastArgument}
  {
  }

  virtual Result run() override
  {
    static const ArgParser::Descriptor descriptors[] = {
        {"--help", nullptr, "show this help message and exit", 0, Arguments::helpSetter},
        {nullptr, "IMAGE", "path to the file system image", 1, Arguments::imageSetter},
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

    Interface * const interface = T::build(arguments.image);
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
  struct Arguments
  {
    const char *image{nullptr};
    const char *entry{nullptr};
    bool help{false};

    static void helpSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->help = true;
    }

    static void imageSetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->image = argument;
    }

    static void entrySetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->entry = argument;
    }
  };
};

#endif // VFS_SHELL_PLATFORM_LINUX_MOUNTSCRIPT_HPP_
