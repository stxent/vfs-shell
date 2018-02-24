/*
 * Core/Vfs/VfsMountpoint.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_VFS_VFSMOUNTPOINT_HPP_
#define VFS_SHELL_CORE_VFS_VFSMOUNTPOINT_HPP_

#include <memory>
#include <xcore/interface.h>
#include "Vfs/Vfs.hpp"

class VfsMountpoint: public VfsNode
{
public:
  VfsMountpoint(const char *name, FsHandle *targetHandle, Interface *targetInterface,
      time64_t timestamp = 0, FsAccess access = FS_ACCESS_READ | FS_ACCESS_WRITE) :
    VfsNode{name, timestamp, access},
    m_targetHandle{targetHandle, [](FsHandle *pointer){ deinit(pointer); }},
    m_targetInterface{targetInterface, [](Interface *pointer){ deinit(pointer); }}
  {
  }

  virtual Result create(const FsFieldDescriptor *descriptors, size_t number) override
  {
    FsNode * const rootNode = static_cast<FsNode *>(fsHandleRoot(m_targetHandle.get()));

    if (rootNode != nullptr)
    {
      const Result res = fsNodeCreate(rootNode, descriptors, number);
      fsNodeFree(rootNode);
      return res;
    }
    else
      return E_ERROR;
  }

  virtual void *head() override
  {
    FsNode * const rootNode = static_cast<FsNode *>(fsHandleRoot(m_targetHandle.get()));

    if (rootNode != nullptr)
    {
      FsNode * const child = static_cast<FsNode *>(fsNodeHead(rootNode));
      fsNodeFree(rootNode);
      return child;
    }
    else
      return nullptr;
  }

  virtual Result remove(FsNode *node) override
  {
    FsNode * const rootNode = static_cast<FsNode *>(fsHandleRoot(m_targetHandle.get()));

    if (rootNode != nullptr)
    {
      const Result res = fsNodeRemove(rootNode, node);
      fsNodeFree(rootNode);
      return res;
    }
    else
      return E_ERROR;
  }

private:
  std::unique_ptr<FsHandle, std::function<void (FsHandle *)>> m_targetHandle;
  std::unique_ptr<Interface, std::function<void (Interface *)>> m_targetInterface;
};

#endif // VFS_SHELL_CORE_VFS_VFSMOUNTPOINT_HPP_
