/*
 * MountScriptBase.cpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <yaf/fat32.h>
#include "Shell/Scripts/MountScriptBase.hpp"
#include "Shell/Settings.hpp"
#include "Shell/ShellHelpers.hpp"
#include "Vfs/VfsMountpoint.hpp"

MountScriptBase::MountScriptBase(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
  ShellScript{parent, firstArgument, lastArgument}
{
}

Result MountScriptBase::mount(const char *dst, Interface *interface)
{
  char path[Settings::PWD_LENGTH];
  ShellHelpers::joinPaths(path, env()["PWD"], dst);

  FsNode * const root = ShellHelpers::openBaseNode(fs(), path);
  if (root == nullptr)
  {
    tty() << name() << ": " << path << ": parent directory not found" << Terminal::EOL;
    return E_ENTRY;
  }

  // Create FAT32 handle
  const Fat32Config config{interface, 4, 2};
  FsHandle * const partition = static_cast<FsHandle *>(init(FatHandle, &config));

  if (partition != nullptr)
  {
    // Create VFS node
    VfsNode * const mountpoint = new VfsMountpoint{ShellHelpers::extractName(dst), partition, interface,
        time().getTime(), FS_ACCESS_READ | FS_ACCESS_WRITE};

    // Link VFS node to the existing file tree
    const FsFieldDescriptor fields[] = {
        {&mountpoint, sizeof(mountpoint), static_cast<FsFieldType>(VfsNode::VFS_NODE_OBJECT)},
    };
    const Result res = fsNodeCreate(root, fields, ARRAY_SIZE(fields));

    if (res != E_OK)
    {
      tty() << name() << ": node linking failed" << Terminal::EOL;
      delete mountpoint;
      deinit(partition);
    }

    fsNodeFree(root);
    return res;
  }
  else
  {
    tty() << name() << ": partition mounting failed" << Terminal::EOL;
    fsNodeFree(root);
    return E_INTERFACE;
  }
}
