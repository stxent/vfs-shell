/*
 * VfsDataNode.cpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Vfs/VfsDataNode.hpp"
#include <cstring>
#include <cstdlib>

VfsDataNode::VfsDataNode(const char *name, time64_t timestamp, FsAccess access) :
  VfsNode{name, timestamp, access},
  m_dataCapacity{0},
  m_dataLength{0},
  m_dataBuffer{nullptr}
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
      if (position <= static_cast<FsLength>(m_dataLength))
      {
        const size_t remaining = static_cast<size_t>(static_cast<FsLength>(m_dataLength) - position);
        const size_t chunk = MIN(remaining, length);

        std::copy(m_dataBuffer.get() + position, m_dataBuffer.get() + position + chunk, static_cast<uint8_t *>(buffer));
        if (read)
          *read = chunk;
        return E_OK;
      }
      else
        return E_VALUE;
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

  auto reallocatedDataBuffer = static_cast<uint8_t *>(malloc(dataCapacity));

  if (reallocatedDataBuffer == nullptr)
    return false;

  if (m_dataLength > 0)
    std::copy(m_dataBuffer.get(), m_dataBuffer.get() + m_dataLength, reallocatedDataBuffer);
  m_dataBuffer.reset(reallocatedDataBuffer);
  m_dataCapacity = dataCapacity;

  return true;
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

  std::copy(bufferPosition, bufferPosition + length, m_dataBuffer.get() + static_cast<size_t>(position));
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

    std::fill(m_dataBuffer.get(), m_dataBuffer.get() + length, static_cast<uint8_t>(fill));
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

    std::copy(position, position + length, m_dataBuffer.get());
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
