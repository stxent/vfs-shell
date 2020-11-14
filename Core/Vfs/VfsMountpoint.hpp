/*
 * Core/Vfs/VfsMountpoint.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_VFS_VFSMOUNTPOINT_HPP_
#define VFS_SHELL_CORE_VFS_VFSMOUNTPOINT_HPP_

#include "Vfs/Vfs.hpp"
#include <xcore/interface.h>
#include <functional>
#include <memory>

class VfsMountpoint: public VfsNode
{
public:
  VfsMountpoint(const char *, FsHandle *, Interface *, time64_t = 0, FsAccess = FS_ACCESS_READ | FS_ACCESS_WRITE);

  virtual Result create(const FsFieldDescriptor *descriptors, size_t number) override;
  virtual void *head() override;
  virtual Result remove(FsNode *node) override;

private:
  std::unique_ptr<FsHandle, std::function<void (FsHandle *)>> m_targetHandle;
  std::unique_ptr<Interface, std::function<void (Interface *)>> m_targetInterface;
};

#endif // VFS_SHELL_CORE_VFS_VFSMOUNTPOINT_HPP_
