/*
 * Core/Shell/Scripts/MountScriptBase.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_MOUNTSCRIPTBASE_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_MOUNTSCRIPTBASE_HPP_

#include <yaf/fat32.h>
#include "Shell/Settings.hpp"
#include "Shell/ShellScript.hpp"
#include "Vfs/VfsMountpoint.hpp"

class MountScriptBase: public ShellScript
{
public:
  MountScriptBase(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    ShellScript{parent, firstArgument, lastArgument}
  {
  }

  static const char *name()
  {
    return "mount";
  }

protected:
  Result mount(const char *dst, Interface *interface)
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

    if (partition == nullptr)
    {
      tty() << name() << ": partition mounting failed" << Terminal::EOL;
      fsNodeFree(root);
      return E_INTERFACE;
    }

    // Create VFS node
    VfsNode * const mountpoint = new VfsMountpoint{ShellHelpers::extractName(dst), partition, interface,
        time().microtime(), FS_ACCESS_READ | FS_ACCESS_WRITE};

    // Link VFS node to the existing file tree
    const FsFieldDescriptor descriptors[] = {
        {&mountpoint, sizeof(mountpoint), static_cast<FsFieldType>(VfsNode::VFS_NODE_OBJECT)},
    };
    const Result res = fsNodeCreate(root, descriptors, ARRAY_SIZE(descriptors));

    fsNodeFree(root);

    if (res != E_OK)
    {
      tty() << name() << ": node linking failed" << Terminal::EOL;
      delete mountpoint;
      deinit(partition);
    }

    return res;
  }
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_MOUNTSCRIPTBASE_HPP_
