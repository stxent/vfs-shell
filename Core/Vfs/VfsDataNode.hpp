/*
 * Core/Vfs/VfsDataNode.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_VFS_VFSDATANODE_HPP_
#define VFS_SHELL_CORE_VFS_VFSDATANODE_HPP_

#include <memory>
#include "Vfs/Vfs.hpp"

class VfsDataNode: public VfsNode
{
public:
  VfsDataNode(const char *name, const void *data, size_t dataLength, time64_t timestamp = 0,
      FsAccess access = FS_ACCESS_READ | FS_ACCESS_WRITE) :
    VfsNode{name, timestamp, access},
    m_dataLength{dataLength},
    m_dataBuffer{std::make_unique<uint8_t []>(m_dataLength)}
  {
    memcpy(m_dataBuffer.get(), data, m_dataLength);
  }

  virtual Result length(FsFieldType type, FsLength *fieldLength)
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

  virtual Result read(FsFieldType type, FsLength position, void *buffer, size_t bufferLength,
      size_t *bytesRead) override
  {
    switch (type)
    {
      case FS_NODE_DATA:
      {
        if (position <= static_cast<FsLength>(m_dataLength))
        {
          const size_t chunkLength = static_cast<size_t>(std::min(static_cast<FsLength>(m_dataLength) - position,
              static_cast<FsLength>(bufferLength)));
          memcpy(buffer, m_dataBuffer.get() + position, chunkLength);
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

  virtual Result write(FsFieldType type, FsLength position, const void *buffer, size_t bufferLength,
      size_t *bytesWritten) override
  {
    switch (type)
    {
      case FS_NODE_DATA:
        return E_INVALID; // TODO

      default:
        return VfsNode::write(type, position, buffer, bufferLength, bytesWritten);
    }
  }

private:
  size_t m_dataLength;
  std::unique_ptr<uint8_t []> m_dataBuffer;
};

#endif // VFS_SHELL_CORE_VFS_VFSDATANODE_HPP_
