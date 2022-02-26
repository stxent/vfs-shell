/*
 * VfsMountpoint.cpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Vfs/VfsMountpoint.hpp"

VfsMountpoint::VfsMountpoint(FsHandle *targetHandle, Interface *targetInterface,
    time64_t timestamp, FsAccess access) :
  VfsNode{timestamp, access},
  m_targetHandle{targetHandle, [](FsHandle *pointer){ deinit(pointer); }},
  m_targetInterface{targetInterface, [](Interface *pointer){ deinit(pointer); }}
{
}

Result VfsMountpoint::create(const FsFieldDescriptor *descriptors, size_t number)
{
  FsNode * const root = static_cast<FsNode *>(fsHandleRoot(m_targetHandle.get()));
  Result res = E_ERROR;

  if (root != nullptr)
  {
    res = fsNodeCreate(root, descriptors, number);
    fsNodeFree(root);
  }

  return res;
}

void *VfsMountpoint::head()
{
  FsNode * const root = static_cast<FsNode *>(fsHandleRoot(m_targetHandle.get()));
  FsNode *child = nullptr;

  if (root != nullptr)
  {
    child = static_cast<FsNode *>(fsNodeHead(root));
    fsNodeFree(root);
  }

  return child;
}

Result VfsMountpoint::remove(FsNode *node)
{
  FsNode * const root = static_cast<FsNode *>(fsHandleRoot(m_targetHandle.get()));
  Result res = E_ERROR;

  if (root != nullptr)
  {
    res = fsNodeRemove(root, node);
    fsNodeFree(root);
  }

  return res;
}
