/*
 * Core/Shell/Scripts/MountScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_MOUNTSCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_MOUNTSCRIPT_HPP_

#include "Shell/ArgParser.hpp"
#include "Shell/Interfaces/InterfaceProxy.hpp"
#include "Shell/Scripts/MountScriptBase.hpp"
#include "Vfs/Vfs.hpp"

class MockMountInterfaceBuilder
{
public:
  MockMountInterfaceBuilder() = delete;
  MockMountInterfaceBuilder(const MockMountInterfaceBuilder &) = delete;
  MockMountInterfaceBuilder &operator=(const MockMountInterfaceBuilder &) = delete;

  static Interface *build(Interface *base)
  {
    const InterfaceProxy::Config config{base};
    const auto interface = static_cast<Interface *>(init(InterfaceProxy, &config));
    return interface;
  }
};

template<typename T = MockMountInterfaceBuilder>
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
        {nullptr, "DEVICE", "search the device for a filesystem", 1, Arguments::deviceSetter},
        {nullptr, "DIR", "attach a filesystem at the specified directory", 1, Arguments::directorySetter}
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

    char path[Settings::PWD_LENGTH];
    fsJoinPaths(path, env()["PWD"], arguments.device);

    FsNode * const device = fsOpenNode(fs(), path);
    if (device == nullptr)
    {
      tty() << name() << ": " << arguments.device << ": node not found" << Terminal::EOL;
      return E_ENTRY;
    }

    // Extract from device
    Interface *base;
    Result res;

    res = fsNodeRead(device, static_cast<FsFieldType>(VfsNode::VFS_NODE_INTERFACE),
        0, &base, sizeof(base), nullptr);
    fsNodeFree(device);

    if (res != E_OK)
    {
      tty() << name() << ": " << arguments.device << ": incorrect device" << Terminal::EOL;
      return E_INTERFACE;
    }

    Interface * const interface = T::build(base);

    if (interface != nullptr)
      res = mount(arguments.directory, interface);
    else
      res = E_INTERFACE;

    if (res != E_OK)
    {
      tty() << name() << ": mount failed" << Terminal::EOL;
    }

    return res;
  }

private:
  struct Arguments
  {
    const char *device{nullptr};
    const char *directory{nullptr};
    bool help{false};

    static void deviceSetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->device = argument;
    }

    static void directorySetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->directory = argument;
    }

    static void helpSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->help = true;
    }
  };
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_MOUNTSCRIPT_HPP_
