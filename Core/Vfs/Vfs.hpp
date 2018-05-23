/*
 * Core/Vfs/Vfs.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_VFS_VFS_HPP_
#define VFS_SHELL_CORE_VFS_VFS_HPP_

#include <cstring>
#include <memory>
#include <new>
#include <xcore/fs.h>
#include <xcore/realtime.h>

extern const FsNodeClass * const VfsNodeProxyClass;

class VfsDirectory;
class VfsHandle;

class VfsNode
{
  friend class VfsNodeProxy; // TODO Remove

public:
  enum VfsFieldType
  {
    VFS_NODE_OBJECT = FS_TYPE_END
  };

  VfsNode(const char *name, time64_t timestamp, FsAccess access) :
    m_handle{nullptr},
    m_parent{nullptr},
    m_timestamp{timestamp},
    m_access{access}
  {
    if (name != nullptr)
    {
      m_name = std::make_unique<char []>(strlen(name) + 1);
      strcpy(m_name.get(), name);
    }
    else
      m_name = nullptr;
  }

  VfsNode(VfsNode &&) = default;
  virtual ~VfsNode() = default;

  virtual Result create(const FsFieldDescriptor *, size_t)
  {
    return E_INVALID;
  }

  virtual VfsNode *fetch(VfsNode *)
  {
    return nullptr;
  }

  virtual void *head()
  {
    return nullptr;
  }

  virtual Result length(FsFieldType type, FsLength *fieldLength)
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

  virtual VfsNode *next()
  {
    return m_parent != nullptr ? m_parent->fetch(this) : nullptr;
  }

  virtual Result read(FsFieldType type, FsLength position, void *buffer, size_t bufferLength, size_t *bytesRead)
  {
    size_t count = 0;

    switch (type)
    {
      case FS_NODE_ACCESS:
      {
        static const FsAccess value = FS_ACCESS_READ | FS_ACCESS_WRITE;

        if (position)
          return E_INVALID;
        if (bufferLength != sizeof(value))
          return E_VALUE;

        memcpy(buffer, &value, sizeof(value));
        count = sizeof(value);
        break;
      }

      case FS_NODE_ID:
      {
        if (position)
          return E_INVALID;
        if (bufferLength != sizeof(void *))
          return E_VALUE;

        void * const pointer = reinterpret_cast<void *>(this);
        memcpy(buffer, &pointer, sizeof(pointer));
        count = sizeof(pointer);
        break;
      }

      case FS_NODE_NAME:
      {
        const size_t nameLength = strlen(m_name.get()) + 1;

        if (position)
          return E_INVALID;
        if (bufferLength < nameLength)
          return E_VALUE;

        strcpy(static_cast<char *>(buffer), m_name.get());
        count = nameLength;
        break;
      }

      case FS_NODE_TIME:
      {
        if (position)
          return E_INVALID;
        if (bufferLength != sizeof(m_timestamp))
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

  virtual Result remove(FsNode *)
  {
    return E_INVALID;
  }

  virtual Result write(FsFieldType type, FsLength position, const void *buffer, size_t bufferLength,
      size_t *bytesWritten)
  {
    size_t count = 0;

    switch (type)
    {
      case FS_NODE_TIME:
      {
        if (position)
          return E_INVALID;
        if (bufferLength != sizeof(m_timestamp))
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

  virtual void enter(VfsHandle *handle, VfsNode *node)
  {
    m_handle = handle;
    m_parent = node;
  }

  virtual void leave()
  {
    m_handle = nullptr;
    m_parent = nullptr;
  }

protected:
  VfsHandle *m_handle;
  VfsNode *m_parent;

  std::unique_ptr<char []> m_name;
  time64_t m_timestamp;
  FsAccess m_access;
};

class VfsNodeProxy
{
public:
  struct Config
  {
    VfsNode *node;
  };

  VfsNodeProxy(const VfsNodeProxy &) = delete;
  VfsNodeProxy &operator=(const VfsNodeProxy &) = delete;

  static Result init(void *object, const void *config)
  {
    new (object) VfsNodeProxy{static_cast<const Config *>(config)->node};
    return E_OK;
  }

  static void deinit(void *object)
  {
    static_cast<VfsNodeProxy *>(object)->~VfsNodeProxy();
  }

  static Result create(void *object, const struct FsFieldDescriptor *descriptors, size_t number)
  {
    return static_cast<VfsNodeProxy *>(object)->createImpl(descriptors, number);
  }

  static void *head(void *object)
  {
    return static_cast<VfsNodeProxy *>(object)->headImpl();
  }

  static void free(void *object)
  {
    return static_cast<VfsNodeProxy *>(object)->freeImpl();
  }

  static Result length(void *object, FsFieldType type, FsLength *fieldLength)
  {
    return static_cast<VfsNodeProxy *>(object)->lengthImpl(type, fieldLength);
  }

  static Result next(void *object)
  {
    return static_cast<VfsNodeProxy *>(object)->nextImpl();
  }

  static Result read(void *object, FsFieldType type, FsLength position, void *buffer, size_t bufferLength,
      size_t *bytesRead)
  {
    return static_cast<VfsNodeProxy *>(object)->readImpl(type, position, buffer, bufferLength, bytesRead);
  }

  static Result remove(void *object, void *node)
  {
    return static_cast<VfsNodeProxy *>(object)->removeImpl(node);
  }

  static Result write(void *object, FsFieldType type, FsLength position, const void *buffer, size_t bufferLength,
      size_t *bytesWritten)
  {
    return static_cast<VfsNodeProxy *>(object)->writeImpl(type, position, buffer, bufferLength, bytesWritten);
  }

  VfsNode *get()
  {
    return m_node;
  }

protected:
  FsNode m_base;
  VfsNode *m_node;

  VfsNodeProxy(VfsNode *node) :
    m_node{node}
  {
  }

  void freeImpl();

  Result createImpl(const struct FsFieldDescriptor *descriptors, size_t number)
  {
    return m_node->create(descriptors, number);
  }

  void *headImpl()
  {
    return m_node->head();
  }

  Result lengthImpl(FsFieldType type, FsLength *length)
  {
    return m_node->length(type, length);
  }

  Result nextImpl()
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

  Result readImpl(FsFieldType type, FsLength position, void *buffer, size_t bufferLength, size_t *bytesRead)
  {
    return m_node->read(type, position, buffer, bufferLength, bytesRead);
  }

  Result removeImpl(void *node)
  {
    return m_node->remove(static_cast<FsNode *>(node));
  }

  Result writeImpl(FsFieldType type, FsLength position, const void *buffer, size_t bufferLength,
      size_t *bytesWritten)
  {
    return m_node->write(type, position, buffer, bufferLength, bytesWritten);
  }
};

#endif // VFS_SHELL_CORE_VFS_VFS_HPP_
