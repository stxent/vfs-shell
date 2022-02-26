/*
 * MountScriptBase.cpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Shell/Scripts/MountScriptBase.hpp"
#include "Shell/Settings.hpp"
#include "Shell/ShellHelpers.hpp"
#include "Vfs/VfsMountpoint.hpp"
#include <xcore/fs/utils.h>
#include <yaf/fat32.h>
#include <new>

MountScriptBase::MountScriptBase(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
  ShellScript{parent, firstArgument, lastArgument}
{
}

Result MountScriptBase::mount(const char *dst, Interface *interface)
{
  char path[Settings::PWD_LENGTH];
  fsJoinPaths(path, env()["PWD"], dst);

  // Create FAT32 handle
  const Fat32Config config{interface, 4, 2};
  FsHandle * const partition = static_cast<FsHandle *>(init(FatHandle, &config));

  if (partition != nullptr)
  {
    // Create VFS node
    const auto mountpoint = static_cast<VfsMountpoint *>(malloc(sizeof(VfsMountpoint)));
    Result res;

    if (mountpoint != nullptr)
    {
      new (mountpoint) VfsMountpoint{partition, interface, time().getTime()};
      res = ShellHelpers::injectNode(fs(), mountpoint, path);

      if (res != E_OK)
        delete mountpoint;
    }
    else
    {
      deinit(partition);
      deinit(interface);
      res = E_MEMORY;
    }

    return res;
  }
  else
  {
    tty() << name() << ": partition mounting failed" << Terminal::EOL;

    deinit(interface);
    return E_INTERFACE;
  }
}
