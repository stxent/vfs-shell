/*
 * VfsDirectory.cpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Vfs/VfsDataNode.hpp"
#include "Vfs/VfsDirectory.hpp"
#include "Vfs/VfsHandle.hpp"
#include <algorithm>
#include <cstring>
#include <new>

VfsDirectory::VfsDirectory(time64_t timestamp, FsAccess access) :
  VfsNode{timestamp, access}
{
}

VfsDirectory::~VfsDirectory()
{
  VfsHandle::Locker locker(*m_handle);

  for (auto iter = m_nodes.begin(); iter != m_nodes.end();)
  {
    VfsNode * const node = *iter;

    iter = m_nodes.erase(iter);

    node->leave();
    delete node;
  }
}

Result VfsDirectory::create(const FsFieldDescriptor *descriptors, size_t number)
{
  if (!(m_access & FS_ACCESS_WRITE))
  {
    /* Write operations are forbidden */
    return E_ACCESS;
  }

  const struct FsFieldDescriptor *dataDesc = 0;
  const struct FsFieldDescriptor *nameDesc = 0;
  VfsNode *node = nullptr;
  time64_t nodeTime = 0;
  FsAccess nodeAccess = FS_ACCESS_READ | FS_ACCESS_WRITE;
  bool create = false;

  for (size_t index = 0; index < number; ++index)
  {
    const FsFieldDescriptor * const desc = descriptors + index;

    switch (desc->type)
    {
      case FS_NODE_ACCESS:
        if (desc->length == sizeof(FsAccess))
        {
          memcpy(&nodeAccess, desc->data, sizeof(FsAccess));
          create = true;
          break;
        }
        else
          return E_VALUE;

      case FS_NODE_DATA:
        dataDesc = desc;
        create = true;
        break;

      case FS_NODE_NAME:
        if (desc->length == strlen(static_cast<const char *>(desc->data)) + 1)
        {
          nameDesc = desc;
          create = true;
          break;
        }
        else
          return E_VALUE;

      case FS_NODE_TIME:
        if (desc->length == sizeof(nodeTime))
        {
          memcpy(&nodeTime, desc->data, sizeof(nodeTime));
          create = true;
          break;
        }
        else
          return E_VALUE;

      default:
        break;
    }

    switch (static_cast<enum VfsFieldType>(desc->type))
    {
      case VFS_NODE_OBJECT:
        if (desc->length == sizeof(VfsNode *))
        {
          node = *static_cast<VfsNode * const *>(desc->data);
          break;
        }
        else
          return E_VALUE;

      default:
        break;
    }
  }

  if (create && node != nullptr)
  {
    // Incorrect list of descriptors: attachment and creation conflict
    return E_INVALID;
  }

  if (node == nullptr)
  {
    if (nameDesc == nullptr)
      return E_VALUE;

    if (dataDesc == nullptr)
    {
      // Create directory node
      node = static_cast<VfsDirectory *>(malloc(sizeof(VfsDirectory)));

      if (node != nullptr)
        new (node) VfsDirectory{nodeTime, nodeAccess};
    }
    else
    {
      // Create data node
      const auto entry = static_cast<VfsDataNode *>(malloc(sizeof(VfsDataNode)));

      if (entry != nullptr)
      {
        new (entry) VfsDataNode{nodeTime, nodeAccess};

        if (entry->reserve(dataDesc->data, dataDesc->length))
          node = entry;
      }
    }

    // Try to allocate name buffer
    if (node != nullptr && !node->rename(static_cast<const char *>(nameDesc->data)))
    {
      delete node;
      node = nullptr;
    }
  }

  if (node != nullptr)
  {
    VfsHandle::Locker locker(*m_handle);

    node->enter(m_handle, this);
    m_nodes.push_back(node);

    return E_OK;
  }
  else
    return E_MEMORY;
}

void *VfsDirectory::head()
{
  if (m_access & FS_ACCESS_READ)
  {
    VfsHandle::Locker locker(*m_handle);

    if (!m_nodes.empty())
      return m_handle->makeNodeProxy(m_nodes.front());
  }

  return nullptr;
}

VfsNode *VfsDirectory::fetch(VfsNode *current)
{
  if (m_access & FS_ACCESS_READ)
  {
    VfsHandle::Locker locker(*m_handle);
    auto iter = std::find(m_nodes.begin(), m_nodes.end(), current);

    if (iter != m_nodes.end() && ++iter != m_nodes.end())
      return *iter;
  }

  return nullptr;
}

Result VfsDirectory::remove(FsNode *proxy)
{
  if (!(m_access & FS_ACCESS_WRITE))
    return E_ACCESS;

  VfsNode * const node = reinterpret_cast<VfsNodeProxy *>(proxy)->get();
  Result res;
  FsAccess access;

  res = node->read(FS_NODE_ACCESS, 0, &access, sizeof(access), nullptr);
  if (res == E_OK)
  {
    if (access & FS_ACCESS_WRITE)
    {
      VfsHandle::Locker locker(*m_handle);
      auto iter = std::find(m_nodes.begin(), m_nodes.end(), node);

      if (iter != m_nodes.end())
      {
        m_nodes.erase(iter);

        node->leave();
        delete node;

        res = E_OK;
      }
      else
        res = E_ENTRY;
    }
    else
      res = E_ACCESS;
  }

  return res;
}
