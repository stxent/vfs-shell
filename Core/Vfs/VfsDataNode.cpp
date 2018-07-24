/*
 * VfsDataNode.cpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Vfs/VfsDataNode.hpp"

VfsDataNode::VfsDataNode(const char *name, const void *data, size_t dataLength, time64_t timestamp, FsAccess access) :
  VfsNode{name, timestamp, access},
  m_dataCapacity{0},
  m_dataLength{0},
  m_dataBuffer{nullptr}
{
  if (dataLength > 0)
  {
    auto * const dataPosition = static_cast<const uint8_t *>(data);

    reallocateDataBuffer(dataLength);
    std::copy(dataPosition, dataPosition + dataLength, m_dataBuffer.get());
    m_dataLength = dataLength;
  }
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

Result VfsDataNode::read(FsFieldType type, FsLength position, void *buffer, size_t bufferLength, size_t *bytesRead)
{
  switch (type)
  {
    case FS_NODE_DATA:
    {
      if (position <= static_cast<FsLength>(m_dataLength))
      {
        const size_t chunkLength = static_cast<size_t>(std::min(static_cast<FsLength>(m_dataLength) - position,
            static_cast<FsLength>(bufferLength)));

        std::copy(m_dataBuffer.get() + position, m_dataBuffer.get() + position + chunkLength,
            static_cast<uint8_t *>(buffer));
        if (bytesRead != nullptr)
          *bytesRead = chunkLength;
        return E_OK;
      }
      else
        return E_VALUE;
    }

    default:
      return VfsNode::read(type, position, buffer, bufferLength, bytesRead);
  }
}

Result VfsDataNode::write(FsFieldType type, FsLength position, const void *buffer, size_t bufferLength,
    size_t *bytesWritten)
{
  switch (type)
  {
    case FS_NODE_DATA:
      return writeData(position, buffer, bufferLength, bytesWritten);

    default:
      return VfsNode::write(type, position, buffer, bufferLength, bytesWritten);
  }
}

void VfsDataNode::reallocateDataBuffer(size_t desiredSize)
{
  if (desiredSize <= m_dataCapacity)
    return;

  if (!m_dataCapacity)
    m_dataCapacity = INITIAL_LENGTH;
  while (m_dataCapacity < desiredSize)
    m_dataCapacity *= 2;

  auto reallocatedDataBuffer = std::make_unique<uint8_t []>(m_dataCapacity);

  if (m_dataLength > 0)
    std::copy(m_dataBuffer.get(), m_dataBuffer.get() + m_dataLength, reallocatedDataBuffer.get());
  m_dataBuffer.swap(reallocatedDataBuffer);
}

Result VfsDataNode::writeData(FsLength position, const void *buffer, size_t bufferLength, size_t *bytesWritten)
{
  auto * const bufferPosition = static_cast<const uint8_t *>(buffer);

  if (position + bufferLength > m_dataCapacity)
  {
    reallocateDataBuffer(position + bufferLength);
    m_dataLength = position + bufferLength;
  }
  std::copy(bufferPosition + position, bufferPosition + position + bufferLength, m_dataBuffer.get());

  if (bytesWritten != nullptr)
    *bytesWritten = bufferLength;

  // TODO Handle memory errors
  return E_OK;
}
