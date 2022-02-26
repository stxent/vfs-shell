/*
 * VfsDataNode.cpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Vfs/VfsDataNode.hpp"
#include <cstdlib>
#include <cstring>

VfsDataNode::VfsDataNode(time64_t timestamp, FsAccess access) :
  VfsNode{timestamp, access},
  m_dataCapacity{0},
  m_dataLength{0},
  m_dataBuffer{nullptr, [](uint8_t pointer[]){ free(pointer); }}
{
  // TODO Locks in read/write operations
}

Result VfsDataNode::length(FsFieldType type, FsLength *fieldLength)
{
  switch (type)
  {
    case FS_NODE_DATA:
      if (fieldLength != nullptr)
        *fieldLength = static_cast<FsLength>(m_dataLength);
      return E_OK;

    default:
      return VfsNode::length(type, fieldLength);
  }
}

Result VfsDataNode::read(FsFieldType type, FsLength position, void *buffer, size_t length, size_t *read)
{
  switch (type)
  {
    case FS_NODE_DATA:
    {
      if (!(m_access & FS_ACCESS_READ))
        return E_ACCESS;
      if (position > static_cast<FsLength>(m_dataLength))
        return E_VALUE;

      const size_t remaining = static_cast<size_t>(static_cast<FsLength>(m_dataLength) - position);
      const size_t chunk = MIN(remaining, length);

      memcpy(buffer, m_dataBuffer.get() + position, chunk);
      if (read)
        *read = chunk;
      return E_OK;
    }

    default:
      return VfsNode::read(type, position, buffer, length, read);
  }
}

Result VfsDataNode::write(FsFieldType type, FsLength position, const void *buffer, size_t length, size_t *written)
{
  switch (type)
  {
    case FS_NODE_DATA:
      if (!(m_access & FS_ACCESS_WRITE))
        return E_ACCESS;

      return writeDataBuffer(position, buffer, length, written);

    default:
      return VfsNode::write(type, position, buffer, length, written);
  }
}

bool VfsDataNode::reallocateDataBuffer(size_t length)
{
  auto dataCapacity = m_dataCapacity;

  if (!dataCapacity)
    dataCapacity = INITIAL_LENGTH;
  while (dataCapacity < length)
    dataCapacity *= 2;

  const auto reallocatedDataBuffer = static_cast<uint8_t *>(malloc(dataCapacity));

  if (reallocatedDataBuffer != nullptr)
  {
    if (m_dataLength > 0)
      memcpy(reallocatedDataBuffer, m_dataBuffer.get(), m_dataLength);

    m_dataBuffer.reset(reallocatedDataBuffer);
    m_dataCapacity = dataCapacity;

    return true;
  }
  else
    return false;
}

Result VfsDataNode::writeDataBuffer(FsLength position, const void *buffer, size_t length, size_t *written)
{
  auto * const bufferPosition = static_cast<const uint8_t *>(buffer);
  const auto end = static_cast<size_t>(position + static_cast<FsLength>(length));

  if (end > m_dataCapacity)
  {
    if (!reallocateDataBuffer(end))
      return E_MEMORY;
  }

  memcpy(m_dataBuffer.get() + static_cast<size_t>(position), bufferPosition, length);
  if (end > m_dataLength)
    m_dataLength = end;

  if (written != nullptr)
    *written = length;

  return E_OK;
}

bool VfsDataNode::reserve(size_t length, char fill)
{
  if (length > 0)
  {
    if (!reallocateDataBuffer(length))
      return false;

    memset(m_dataBuffer.get(), fill, length);
    m_dataLength = length;
  }
  else
  {
    m_dataCapacity = 0;
    m_dataLength = 0;
    m_dataBuffer.reset();
  }

  return true;
}

bool VfsDataNode::reserve(const void *data, size_t length)
{
  if (length > 0)
  {
    auto * const position = static_cast<const uint8_t *>(data);

    if (!reallocateDataBuffer(length))
      return false;

    memcpy(m_dataBuffer.get(), position, length);
    m_dataLength = length;
  }
  else
  {
    m_dataCapacity = 0;
    m_dataLength = 0;
    m_dataBuffer.reset();
  }

  return true;
}

bool VfsDataNode::reserve(const char *text)
{
  return reserve(text, text != nullptr ? strlen(text) : 0);
}
