/*
 * Core/Shell/Interfaces/InterfaceNode.hpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_INTERFACES_INTERFACENODE_HPP_
#define VFS_SHELL_CORE_SHELL_INTERFACES_INTERFACENODE_HPP_

#include <tuple>
#include <xcore/interface.h>
#include "Shell/Interfaces/InterfaceParameters.hpp"
#include "Shell/Interfaces/SerialParameters.hpp"
#include "Vfs/Vfs.hpp"
#include "Vfs/VfsHandle.hpp"

template<typename T, typename U, T ID>
struct ParamDesc
{
  using Type = U;
  static constexpr T id = ID;

  static constexpr const char *name()
  {
    return ifParamToName<ID>();
  }
};

template<class T>
class InterfaceParamNode: public VfsNode
{
  using Type = typename T::Type;
  static constexpr FsLength TYPE_LENGTH = static_cast<FsLength>(sizeof(Type));

public:
  InterfaceParamNode(const char *name, Interface *interface,
      time64_t timestamp = 0, FsAccess access = FS_ACCESS_READ | FS_ACCESS_WRITE) :
    VfsNode{name, timestamp, access},
    m_interface{interface}
  {
  }

  virtual Result length(FsFieldType type, FsLength *fieldLength) override
  {
    switch (type)
    {
      case FS_NODE_DATA:
        if (fieldLength != nullptr)
          *fieldLength = 0;
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
        Result res;

        if (!position)
        {
          Type value;

          if ((res = ifGetParam(m_interface, static_cast<IfParameter>(T::id), &value)) == E_OK)
          {
            char serializedValue[TerminalHelpers::serializedValueLength<Type>() + 2];
            char * const serializedValueEnd = TerminalHelpers::serialize(serializedValue, value, "\r\n");
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
        const auto value = TerminalHelpers::str2int<Type>(static_cast<const char *>(buffer), bufferLength, &count);

        if (count > 0)
        {
          const Result res = ifSetParam(m_interface, static_cast<IfParameter>(T::id), &value);

          if (res == E_OK && bytesWritten != nullptr)
            *bytesWritten = count;

          return res;
        }
        else
        {
          // Silently accept all nonsensical data to work correctly with output of echo-like scripts
          return E_OK;
        }
      }

      default:
        return VfsNode::write(type, position, buffer, bufferLength, bytesWritten);
    }
  }

private:
  Interface * const m_interface;
  Type cache;
};

template<typename... ARGs>
class InterfaceNode: public VfsNode
{
  template<std::size_t N = 0>
  typename std::enable_if_t<N == sizeof...(ARGs)> updateLinksImpl(VfsHandle *, VfsNode *)
  {
  }

  template<std::size_t N = 0>
  typename std::enable_if_t<N < sizeof...(ARGs)> updateLinksImpl(VfsHandle *handle, VfsNode *node)
  {
    std::get<N>(m_parameters).enter(handle, node);
    updateLinksImpl<N + 1>(handle, node);
  }

  template<std::size_t N = 0>
  typename std::enable_if_t<N == sizeof...(ARGs) - 1, VfsNode *> fetchImpl(VfsNode *)
  {
    return nullptr;
  }

  template<std::size_t N = 0>
  typename std::enable_if_t<N < sizeof...(ARGs) - 1, VfsNode *> fetchImpl(VfsNode *current)
  {
    return &std::get<N>(m_parameters) == current ? &std::get<N + 1>(m_parameters) : fetchImpl<N + 1>(current);
  }

public:
  InterfaceNode(const char *name, Interface *interface, time64_t timestamp = 0,
      FsAccess access = FS_ACCESS_READ | FS_ACCESS_WRITE) :
    VfsNode{name, timestamp, access},
    m_parameters{InterfaceParamNode<ARGs>{ARGs::name(), interface, timestamp, access}...}
  {
    static_assert(sizeof...(ARGs) > 0, "No parameter nodes defined");
  }

  virtual void *head() override
  {
    VfsHandle::Locker locker(*m_handle);
    return m_handle->makeNodeProxy(&std::get<0>(m_parameters));
  }

  virtual VfsNode *fetch(VfsNode *current) override
  {
    return fetchImpl(current);
  }

  virtual void enter(VfsHandle *handle, VfsNode *node) override
  {
    VfsNode::enter(handle, node);
    updateLinksImpl(handle, this);
  }

  virtual void leave() override
  {
    VfsNode::leave();
    updateLinksImpl(nullptr, nullptr);
  }

private:
  std::tuple<InterfaceParamNode<ARGs>...> m_parameters;
};

#endif // VFS_SHELL_CORE_SHELL_INTERFACES_INTERFACENODE_HPP_
