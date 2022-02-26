/*
 * Vfs.cpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Vfs/Vfs.hpp"
#include "Vfs/VfsHandle.hpp"
#include <cstring>
#include <cstdlib>

const FsNodeClass VfsNodeProxy::table{
    sizeof(VfsNodeProxy), // size
    VfsNodeProxy::init,   // init
    VfsNodeProxy::deinit, // deinit

    VfsNodeProxy::create, // create
    VfsNodeProxy::head,   // head
    VfsNodeProxy::free,   // free
    VfsNodeProxy::length, // length
    VfsNodeProxy::next,   // next
    VfsNodeProxy::read,   // read
    VfsNodeProxy::remove, // remove
    VfsNodeProxy::write   // write
};
const FsNodeClass * const VfsNodeProxyClass = &VfsNodeProxy::table;

VfsNode::VfsNode(time64_t timestamp, FsAccess access) :
  m_handle{nullptr},
  m_parent{nullptr},
  m_name{nullptr, [](char pointer[]){ free(pointer); }},
  m_timestamp{timestamp},
  m_access{access}
{
}

Result VfsNode::create(const FsFieldDescriptor *, size_t)
{
  return E_INVALID;
}

VfsNode *VfsNode::fetch(VfsNode *)
{
  return nullptr;
}

void *VfsNode::head()
{
  return nullptr;
}

Result VfsNode::length(FsFieldType type, FsLength *fieldLength)
{
  size_t len = 0;

  switch (type)
  {
    case FS_NODE_ACCESS:
      len = sizeof(FsAccess);
      break;

    case FS_NODE_ID:
      len = sizeof(void *);
      break;

    case FS_NODE_NAME:
      len = strlen(m_name.get()) + 1;
      break;

    case FS_NODE_TIME:
      len = sizeof(time64_t);
      break;

    default:
      return E_INVALID;
  }

  if (fieldLength != nullptr)
    *fieldLength = static_cast<FsLength>(len);
  return E_OK;
}

VfsNode *VfsNode::next()
{
  return m_parent != nullptr ? m_parent->fetch(this) : nullptr;
}

Result VfsNode::read(FsFieldType type, FsLength position, void *buffer, size_t bufferLength, size_t *bytesRead)
{
  size_t count = 0;

  switch (type)
  {
    case FS_NODE_ACCESS:
    {
      if (position || bufferLength != sizeof(m_access))
        return E_VALUE;

      memcpy(buffer, &m_access, sizeof(m_access));
      count = sizeof(m_access);
      break;
    }

    case FS_NODE_ID:
    {
      if (position || bufferLength != sizeof(void *))
        return E_VALUE;

      void * const pointer = reinterpret_cast<void *>(this);
      memcpy(buffer, &pointer, sizeof(pointer));
      count = sizeof(pointer);
      break;
    }

    case FS_NODE_NAME:
    {
      const size_t nameLength = strlen(m_name.get()) + 1;

      if (position || bufferLength < nameLength)
        return E_VALUE;

      strcpy(static_cast<char *>(buffer), m_name.get());
      count = nameLength;
      break;
    }

    case FS_NODE_TIME:
    {
      if (position || bufferLength != sizeof(m_timestamp))
        return E_VALUE;

      memcpy(buffer, &m_timestamp, sizeof(m_timestamp));
      count = sizeof(m_timestamp);
      break;
    }

    default:
      return E_INVALID;
  }

  if (bytesRead != nullptr)
    *bytesRead = static_cast<FsLength>(count);
  return E_OK;
}

Result VfsNode::remove(FsNode *)
{
  return E_INVALID;
}

Result VfsNode::write(FsFieldType type, FsLength position, const void *buffer, size_t bufferLength,
    size_t *bytesWritten)
{
  size_t count = 0;

  switch (type)
  {
    case FS_NODE_ACCESS:
    {
      if (position || bufferLength != sizeof(m_access))
        return E_VALUE;

      memcpy(&m_access, buffer, sizeof(m_access));
      count = sizeof(m_access);
      break;
    }

    case FS_NODE_TIME:
    {
      if (position || bufferLength != sizeof(m_timestamp))
        return E_VALUE;

      memcpy(&m_timestamp, buffer, sizeof(m_timestamp));
      count = sizeof(m_timestamp);
      break;
    }

    default:
      return E_INVALID;
  }

  if (bytesWritten != nullptr)
    *bytesWritten = count;
  return E_OK;
}

void VfsNode::enter(VfsHandle *handle, VfsNode *node)
{
  m_handle = handle;
  m_parent = node;
}

void VfsNode::leave()
{
  m_handle = nullptr;
  m_parent = nullptr;
}

bool VfsNode::rename(const char *name)
{
  if (name != nullptr)
  {
    const auto reallocatedNameBuffer = static_cast<char *>(malloc(strlen(name) + 1));

    if (reallocatedNameBuffer != nullptr)
    {
      strcpy(reallocatedNameBuffer, name);
      m_name.reset(reallocatedNameBuffer);

      return true;
    }
    else
      return false;
  }
  else
  {
    m_name.reset(nullptr);
    return true;
  }
}

void VfsNode::operator delete(void *pointer)
{
  free(pointer);
}

VfsNodeProxy::VfsNodeProxy(VfsNode *node) :
  m_node{node}
{
}

Result VfsNodeProxy::createImpl(const struct FsFieldDescriptor *descriptors, size_t number)
{
  return m_node->create(descriptors, number);
}

void VfsNodeProxy::freeImpl()
{
  return m_node->m_handle->freeNodeProxy(this);
}

void *VfsNodeProxy::headImpl()
{
  return m_node->head();
}

Result VfsNodeProxy::lengthImpl(FsFieldType type, FsLength *length)
{
  return m_node->length(type, length);
}

Result VfsNodeProxy::nextImpl()
{
  VfsNode * const nextNode = m_node->next();

  if (nextNode != nullptr)
  {
    m_node = nextNode;
    return E_OK;
  }
  else
    return E_ENTRY;
}

Result VfsNodeProxy::readImpl(FsFieldType type, FsLength position, void *buffer, size_t bufferLength,
    size_t *bytesRead)
{
  return m_node->read(type, position, buffer, bufferLength, bytesRead);
}

Result VfsNodeProxy::removeImpl(void *node)
{
  return m_node->remove(static_cast<FsNode *>(node));
}

Result VfsNodeProxy::writeImpl(FsFieldType type, FsLength position, const void *buffer, size_t bufferLength,
    size_t *bytesWritten)
{
  return m_node->write(type, position, buffer, bufferLength, bytesWritten);
}
