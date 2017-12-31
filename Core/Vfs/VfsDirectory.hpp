/*
 * Core/Vfs/VfsDirectory.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_VFS_VFSDIRECTORY_HPP_
#define VFS_SHELL_CORE_VFS_VFSDIRECTORY_HPP_

#include <list>
#include "Vfs/Vfs.hpp"

class VfsHandle;

class VfsDirectory: public VfsNode
{
public:
  VfsDirectory(const char *name, time64_t timestamp = 0, FsAccess access = FS_ACCESS_READ | FS_ACCESS_WRITE) :
    VfsNode{name, timestamp, access}
  {
  }

  virtual Result create(const FsFieldDescriptor *, size_t) override;
  virtual VfsNode *fetch(VfsNode *current) override;
  virtual void *head() override;
  virtual Result remove(VfsNode *) override;

private:
  // List of descendant nodes
  std::list<VfsNode *> m_nodes;
};

#endif // VFS_SHELL_CORE_VFS_VFSDIRECTORY_HPP_
