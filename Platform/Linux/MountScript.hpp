/*
 * Core/Shell/Scripts/MountScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_PLATFORM_LINUX_MOUNTSCRIPT_HPP_
#define VFS_SHELL_PLATFORM_LINUX_MOUNTSCRIPT_HPP_

#include "Shell/Scripts/MountScriptBase.hpp"

template<class T>
class MountScript: public MountScriptBase
{
public:
  MountScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    MountScriptBase{parent, firstArgument, lastArgument}
  {
  }

  virtual Result run() override
  {
    Arguments arguments;
    const std::array<ArgParser::Descriptor, 2> descriptors = {
        {
            {"--help", "print help message", 0, boolArgumentSetter, &arguments.showHelpMessage},
            {nullptr, nullptr, 2, positionalArgumentParser, &arguments}
        }
    };

    ArgParser::parse(m_firstArgument, m_lastArgument, descriptors.begin(), descriptors.end());
    if (arguments.src == nullptr || arguments.dst == nullptr || arguments.showHelpMessage)
    {
      ArgParser::help(tty(), descriptors.begin(), descriptors.end());
      return E_OK;
    }

    Interface * const interface = T::build(arguments.src);
    if (interface == nullptr)
    {
      tty() << name() << ": file not found" << Terminal::EOL;
      return E_VALUE;
    }

    const Result res = mount(arguments.dst, interface);
    if (res != E_OK)
      deinit(interface);

    return res;
  }

private:
  struct Arguments
  {
    Arguments() :
      src{nullptr},
      dst{nullptr},
      showHelpMessage{false}
    {
    }

    const char *src;
    const char *dst;
    bool showHelpMessage;
  };

  static void boolArgumentSetter(void *object, const char *)
  {
    *static_cast<bool *>(object) = true;
  }

  static void positionalArgumentParser(void *object, const char *argument)
  {
    auto * const args = static_cast<Arguments *>(object);

    if (args->src == nullptr)
      args->src = argument;
    else if (args->dst == nullptr)
      args->dst = argument;
    else
      args->showHelpMessage = true; // Incorrect argument count
  }
};

#endif // VFS_SHELL_PLATFORM_LINUX_MOUNTSCRIPT_HPP_
