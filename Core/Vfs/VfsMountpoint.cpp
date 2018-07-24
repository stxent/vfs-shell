/*
 * VfsMountpoint.cpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Vfs/VfsMountpoint.hpp"

VfsMountpoint::VfsMountpoint(const char *name, FsHandle *targetHandle, Interface *targetInterface,
    time64_t timestamp, FsAccess access) :
  VfsNode{name, timestamp, access},
  m_targetHandle{targetHandle, [](FsHandle *pointer){ deinit(pointer); }},
  m_targetInterface{targetInterface, [](Interface *pointer){ deinit(pointer); }}
{
}

Result VfsMountpoint::create(const FsFieldDescriptor *descriptors, size_t number)
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

void *VfsMountpoint::head()
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

Result VfsMountpoint::remove(FsNode *node)
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
