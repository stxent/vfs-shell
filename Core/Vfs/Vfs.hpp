/*
 * Core/Vfs/Vfs.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_VFS_VFS_HPP_
#define VFS_SHELL_CORE_VFS_VFS_HPP_

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

  VfsNode(const char *, time64_t, FsAccess);
  VfsNode(VfsNode &&) = default;
  virtual ~VfsNode() = default;

  virtual Result create(const FsFieldDescriptor *, size_t);
  virtual VfsNode *fetch(VfsNode *);
  virtual void *head();
  virtual Result length(FsFieldType, FsLength *);
  virtual VfsNode *next();
  virtual Result read(FsFieldType, FsLength, void *, size_t, size_t *);
  virtual Result remove(FsNode *);
  virtual Result write(FsFieldType, FsLength, const void *, size_t, size_t *);

  virtual void enter(VfsHandle *, VfsNode *);
  virtual void leave();

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

  VfsNodeProxy(VfsNode *);

  Result createImpl(const struct FsFieldDescriptor *, size_t);
  void freeImpl();
  void *headImpl();
  Result lengthImpl(FsFieldType, FsLength *);
  Result nextImpl();
  Result readImpl(FsFieldType, FsLength, void *, size_t, size_t *);
  Result removeImpl(void *);
  Result writeImpl(FsFieldType, FsLength, const void *, size_t, size_t *);
};

#endif // VFS_SHELL_CORE_VFS_VFS_HPP_
