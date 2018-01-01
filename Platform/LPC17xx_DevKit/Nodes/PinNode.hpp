/*
 * Platform/LPC17xx_DevKit/Nodes/PinNode.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_NODES_PINNODE_HPP_
#define VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_NODES_PINNODE_HPP_

#include <halm/pin.h>
#include "Vfs/Vfs.hpp"

class PinNode: public VfsNode
{
public:
  PinNode(const char *name, long port, long pin, bool value, time64_t timestamp = 0,
      FsAccess access = FS_ACCESS_READ | FS_ACCESS_WRITE) :
    VfsNode{name, timestamp, access},
    m_pin{pinInit(PIN(port, pin))}
  {
    if (pinValid(m_pin))
      pinOutput(m_pin, value);
  }

  PinNode(const char *name, long port, long pin, time64_t timestamp = 0,
      FsAccess access = FS_ACCESS_READ) :
    VfsNode{name, timestamp, static_cast<FsAccess>(access & ~FS_ACCESS_WRITE)},
    m_pin{pinInit(PIN(port, pin))}
  {
    if (pinValid(m_pin))
      pinInput(m_pin);
  }

  virtual Result length(FsFieldType type, FsLength *fieldLength)
  {
    switch (type)
    {
      case FS_NODE_DATA:
        if (pinValid(m_pin))
        {
          if (fieldLength != nullptr)
            *fieldLength = 1;
          return E_OK;
        }
        else
          return E_INVALID;

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
        if (pinValid(m_pin))
        {
          if (bufferLength)
          {
            // TODO Rewrite
            if (!position)
            {
              *static_cast<char *>(buffer) = pinRead(m_pin) ? '1' : '0';
              if (bytesRead != nullptr)
                *bytesRead = 1;
            }
            else
            {
              if (bytesRead != nullptr)
                *bytesRead = 0;
            }
            return E_OK;
          }
          else
            return E_VALUE;
        }
        else
          return E_INVALID;
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
        if (pinValid(m_pin))
        {
          if (bufferLength)
          {
            const char value = *static_cast<const char *>(buffer);

            if (value == '0' || (bufferLength >= 5 && !memcmp(buffer, "false", 5))
                || (bufferLength >= 3 && !memcmp(buffer, "off", 3)))
            {
              pinReset(m_pin);
            }
            else if (value == '1' || (bufferLength >= 4 && !memcmp(buffer, "true", 5))
                || (bufferLength >= 2 && !memcmp(buffer, "on", 2)))
            {
              pinSet(m_pin);
            }

            if (bytesWritten != nullptr)
              *bytesWritten = bufferLength;
            return E_OK;
          }
          else
            return E_VALUE;
        }
        else
          return E_INVALID;

      default:
        return VfsNode::write(type, position, buffer, bufferLength, bytesWritten);
    }
  }

private:
  const Pin m_pin;
};

#endif // VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_NODES_PINNODE_HPP_
