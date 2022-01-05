/*
 * Platform/LPC17xx_DevKit/Nodes/DacNode.hpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_NODES_DACNODE_HPP_
#define VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_NODES_DACNODE_HPP_

#include "Vfs/Vfs.hpp"
#include <halm/platform/lpc/dac.h>
#include <memory>

class DacNode: public VfsNode
{
public:
  DacNode(const char *name, time64_t timestamp = 0, FsAccess access = FS_ACCESS_READ | FS_ACCESS_WRITE) :
    VfsNode{name, timestamp, access},
    cache{0}
  {
    static const DacConfig config{cache, PIN(0, 26)};
    m_interface = {static_cast<Interface *>(init(Dac, &config)), [](Interface *pointer){ deinit(pointer); }};
  }

  virtual Result read(FsFieldType type, FsLength position, void *buffer, size_t bufferLength,
      size_t *bytesRead) override
  {
    switch (type)
    {
      case FS_NODE_DATA:
      {
        Result res;

        if (!position)
        {
          char serializedValue[TerminalHelpers::serializedValueLength<uint16_t>() + 2];
          char * const serializedValueEnd = TerminalHelpers::serialize(serializedValue, cache, "\r\n");
          const size_t requiredBufferLength = serializedValueEnd - serializedValue;

          if (requiredBufferLength < bufferLength)
          {
            memcpy(buffer, serializedValue, requiredBufferLength);
            if (bytesRead != nullptr)
              *bytesRead = requiredBufferLength;
            res = E_OK;
          }
          else
            res = E_FULL;
        }
        else
          res = E_EMPTY;

        return res;
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
      {
        size_t count;
        const auto value = TerminalHelpers::str2int<uint16_t>(static_cast<const char *>(buffer), bufferLength, &count);

        if (count > 0)
        {
          const Result res = ifWrite(m_interface.get(), &value, sizeof(value)) == sizeof(value) ? E_OK : E_INTERFACE;

          if (res == E_OK && bytesWritten != nullptr)
            *bytesWritten = count;

          return res;
        }
        else
          return E_OK;
      }

      default:
        return VfsNode::write(type, position, buffer, bufferLength, bytesWritten);
    }
  }

private:
  uint16_t cache;
  std::unique_ptr<Interface, std::function<void (Interface *)>> m_interface;
};

#endif // VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_NODES_DACNODE_HPP_
