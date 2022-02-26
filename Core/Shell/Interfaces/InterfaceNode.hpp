/*
 * Core/Shell/Interfaces/InterfaceNode.hpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_INTERFACES_INTERFACENODE_HPP_
#define VFS_SHELL_CORE_SHELL_INTERFACES_INTERFACENODE_HPP_

#include "Shell/Interfaces/DisplayParameters.hpp"
#include "Shell/Interfaces/InterfaceParameters.hpp"
#include "Shell/Interfaces/SerialParameters.hpp"
#include "Shell/TerminalHelpers.hpp"
#include "Vfs/Vfs.hpp"
#include "Vfs/VfsHandle.hpp"
#include <xcore/interface.h>
#include <cassert>
#include <tuple>

template<typename T, typename... ARGs>
class ParamBuilder
{
private:
  using Indices = std::make_index_sequence<sizeof...(ARGs)>;

public:
  static T make(const std::tuple<ARGs...> &args)
  {
    return makeImpl(args, Indices{});
  }

private:
  template<size_t... N>
  static T makeImpl(const std::tuple<ARGs...> &args, std::index_sequence<N...>)
  {
    return T{std::get<N>(args)...};
  }
};

template<typename... ARGs>
struct ParamParser
{
  static auto parse(const char *, size_t, size_t *)
  {
    return std::tuple<>{};
  }
};

template<typename T, typename... ARGs>
struct ParamParser<T, ARGs...>
{
  static auto parse(const char *buffer, size_t length, size_t *converted)
  {
    size_t count = 0;
    const T value = TerminalHelpers::str2int<T>(buffer, length, &count);

    if (converted != nullptr)
      *converted += count;

    return std::tuple_cat(std::make_tuple(value),
        ParamParser<ARGs...>::parse(buffer + count, length - count, converted));
  }
};

template<size_t OFFSET, typename BASE, typename... ARGs>
struct ParamExtractor
{
  static size_t serialize(const BASE &, char *output, size_t length)
  {
    if (length >= 2)
    {
      output[0] = '\r';
      output[1] = '\n';
      return 2;
    }
    else
      return 0;
  }
};

template<size_t OFFSET, typename BASE, typename T, typename... ARGs>
struct ParamExtractor<OFFSET, BASE, T, ARGs...>
{
  static size_t serialize(const BASE &input, char *output, size_t length)
  {
    static constexpr size_t PREFIX_LENGTH{OFFSET > 0 ? 1 : 0};
    static constexpr size_t MAX_VALUE_LENGTH{TerminalHelpers::serializedValueLength<T>()};

    if (PREFIX_LENGTH + MAX_VALUE_LENGTH <= length)
    {
      if (PREFIX_LENGTH)
        *output++ = ' ';

      const T value = *reinterpret_cast<const T *>(reinterpret_cast<const uint8_t *>(&input) + OFFSET);
      char * const endOfChunk = TerminalHelpers::serialize(output, value);
      const size_t currentChunkLength = PREFIX_LENGTH + (endOfChunk - output);
      const size_t nextChunkLength = ParamExtractor<OFFSET + sizeof(T), BASE, ARGs...>::serialize(input,
          endOfChunk, length - currentChunkLength);

      return nextChunkLength ? currentChunkLength + nextChunkLength : 0;
    }
    else
      return 0;
  }
};

template<typename U, U ID, typename T, typename... Ts>
struct ParamDesc
{
  using Type = T;
  static constexpr U id{ID};

  static constexpr const char *name()
  {
    return ifParamToName<ID>();
  }

  static T pack(const char *buffer, size_t length, size_t *converted)
  {
    return packImpl<sizeof...(Ts)>(buffer, length, converted);
  }

  static size_t unpack(const T &input, char *output, size_t length)
  {
    return unpackImpl<sizeof...(Ts)>(input, output, length);
  }

private:
  template<size_t N>
  static typename std::enable_if_t<N == 0, T> packImpl(const char *buffer, size_t length, size_t *converted)
  {
    return TerminalHelpers::str2int<T>(buffer, length, converted);
  }

  template<size_t N>
  static typename std::enable_if_t<N != 0, T> packImpl(const char *buffer, size_t length, size_t *converted)
  {
    if (converted != nullptr)
      *converted = 0;

    return ParamBuilder<T, Ts...>::make(ParamParser<Ts...>::parse(buffer, length, converted));
  }

  template<size_t N>
  static typename std::enable_if_t<N == 0, size_t> unpackImpl(const T &input, char *output, size_t length)
  {
    return ParamExtractor<0, T, T>::serialize(input, output, length);
  }

  template<size_t N>
  static typename std::enable_if_t<N != 0, size_t> unpackImpl(const T &input, char *output, size_t length)
  {
    return ParamExtractor<0, T, Ts...>::serialize(input, output, length);
  }
};

template<typename T>
class InterfaceParamNode: public VfsNode
{
  using Type = typename T::Type;
  static constexpr FsLength TYPE_LENGTH{static_cast<FsLength>(sizeof(Type))};

public:
  InterfaceParamNode(const char *name, Interface *interface,
      time64_t timestamp = 0, FsAccess access = FS_ACCESS_READ | FS_ACCESS_WRITE) :
    VfsNode{timestamp, access},
    m_interface{interface}
  {
    rename(name);
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
        if (!(m_access & FS_ACCESS_READ))
          return E_ACCESS;
        if (position)
          return E_EMPTY;

        Result res;
        Type value;

        if ((res = ifGetParam(m_interface, static_cast<IfParameter>(T::id), &value)) == E_OK)
        {
          const auto count = T::unpack(value, static_cast<char *>(buffer), bufferLength);

          if (count > 0)
          {
            if (bytesRead != nullptr)
              *bytesRead = count;
            res = E_OK;
          }
          else
            res = E_VALUE;
        }

        return res;
      }

      default:
        break;
    }

    return VfsNode::read(type, position, buffer, bufferLength, bytesRead);
  }

  virtual Result write(FsFieldType type, FsLength position, const void *buffer, size_t bufferLength,
      size_t *bytesWritten) override
  {
    switch (type)
    {
      case FS_NODE_DATA:
      {
        if (!(m_access & FS_ACCESS_WRITE))
          return E_ACCESS;
        if (position)
          return E_FULL;

        size_t count;
        const auto value = T::pack(static_cast<const char *>(buffer), bufferLength, &count);

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
          if (bytesWritten != nullptr)
            *bytesWritten = count;

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

  template<std::size_t N>
  typename std::enable_if_t<N == 0, void *> headImpl()
  {
    return nullptr;
  }

  template<std::size_t N>
  typename std::enable_if_t<N != 0, void *> headImpl()
  {
    if (m_access & FS_ACCESS_READ)
    {
      VfsHandle::Locker locker(*m_handle);
      return m_handle->makeNodeProxy(&std::get<0>(m_parameters));
    }

    return nullptr;
  }

  template<std::size_t N = 0>
  typename std::enable_if_t<N + 1 >= sizeof...(ARGs), VfsNode *> fetchImpl(VfsNode *)
  {
    return nullptr;
  }

  template<std::size_t N = 0>
  typename std::enable_if_t<N + 1 < sizeof...(ARGs), VfsNode *> fetchImpl(VfsNode *current)
  {
    if (m_access & FS_ACCESS_READ)
    {
      if (&std::get<N>(m_parameters) == current)
        return &std::get<N + 1>(m_parameters);
      else
        return fetchImpl<N + 1>(current);
    }

    return nullptr;
  }

public:
  InterfaceNode(Interface *interface, time64_t timestamp = 0,
      FsAccess access = FS_ACCESS_READ | FS_ACCESS_WRITE) :
    VfsNode{timestamp, access},
    m_parameters{InterfaceParamNode<ARGs>{ARGs::name(), interface, timestamp, access}...},
    m_interface{interface}
  {
    uint64_t tmp;

    size32 = ifGetParam(m_interface, IF_SIZE, &tmp) == E_OK;
    size64 = ifGetParam(m_interface, IF_SIZE_64, &tmp) == E_OK;

    updateLinksImpl(nullptr, this);
  }

  virtual void *head() override
  {
    return headImpl<sizeof...(ARGs)>();
  }

  virtual VfsNode *fetch(VfsNode *current) override
  {
    return fetchImpl<0>(current);
  }

  virtual void enter(VfsHandle *handle, VfsNode *node) override
  {
    VfsNode::enter(handle, node);
    updateLinksImpl(handle, this);
  }

  virtual void leave() override
  {
    VfsNode::leave();
    updateLinksImpl(nullptr, this);
  }

  virtual Result read(FsFieldType type, FsLength position, void *buffer, size_t bufferLength,
      size_t *bytesRead) override
  {
    switch (static_cast<VfsNode::VfsFieldType>(type))
    {
      case VFS_NODE_INTERFACE:
      {
        if (position || bufferLength != sizeof(m_interface))
          return E_VALUE;

        memcpy(buffer, &m_interface, sizeof(m_interface));
        if (bytesRead != nullptr)
          *bytesRead = sizeof(m_interface);
        return E_OK;
      }

      default:
        break;
    }

    Result res = E_OK;

    switch (type)
    {
      case FS_NODE_DATA:
      {
        if (!(m_access & FS_ACCESS_WRITE))
        {
          res = E_ACCESS;
          break;
        }

        if (size64)
          res = setInterfacePosition<uint64_t>(position);
        else if (size32)
          res = setInterfacePosition<uint32_t>(position);

        if (res != E_OK)
          break;

        const auto count = ifRead(m_interface, buffer, bufferLength);

        if (bytesRead != nullptr)
          *bytesRead = count;

        res = (bufferLength == 0 || count > 0) ? E_OK : E_EMPTY;
        break;
      }

      default:
        res = VfsNode::read(type, position, buffer, bufferLength, bytesRead);
        break;
    }

    return res;
  }

  virtual Result write(FsFieldType type, FsLength position, const void *buffer, size_t bufferLength,
      size_t *bytesWritten) override
  {
    Result res = E_OK;

    switch (type)
    {
      case FS_NODE_DATA:
      {
        if (!(m_access & FS_ACCESS_WRITE))
        {
          res = E_ACCESS;
          break;
        }

        if (size64)
          res = setInterfacePosition<uint64_t>(position);
        else if (size32)
          res = setInterfacePosition<uint32_t>(position);

        if (res != E_OK)
          break;

        const auto count = ifWrite(m_interface, buffer, bufferLength);

        if (bytesWritten != nullptr)
          *bytesWritten = count;

        res = (bufferLength == 0 || count > 0) ? E_OK : E_FULL;
        break;
      }

      default:
        res = VfsNode::write(type, position, buffer, bufferLength, bytesWritten);
        break;
    }

    return res;
  }

private:
  std::tuple<InterfaceParamNode<ARGs>...> m_parameters;
  Interface *m_interface;
  bool size32;
  bool size64;

  template<typename T>
  Result setInterfacePosition(FsLength position)
  {
    static constexpr IfParameter POSITION_PARAM = std::is_same_v<T, uint32_t> ? IF_POSITION : IF_POSITION_64;

    const T pos = static_cast<T>(position);
    return ifSetParam(m_interface, POSITION_PARAM, &pos);
  }
};

#endif // VFS_SHELL_CORE_SHELL_INTERFACES_INTERFACENODE_HPP_
