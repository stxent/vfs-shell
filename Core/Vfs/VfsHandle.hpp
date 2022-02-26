/*
 * Core/Vfs/VfsHandle.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_VFS_VFSHANDLE_HPP_
#define VFS_SHELL_CORE_VFS_VFSHANDLE_HPP_

#include "Vfs/VfsDirectory.hpp"

extern const FsHandleClass * const VfsHandleClass;

class VfsHandle
{
public:
  class Locker
  {
  public:
    Locker(const Locker &) = delete;
    Locker &operator=(const Locker &) = delete;

    Locker(VfsHandle &handle) :
      m_handle{handle}
    {
      m_handle.lock();
    }

    ~Locker()
    {
      m_handle.unlock();
    }

  private:
    VfsHandle &m_handle;
  };

  static const FsHandleClass table;

  VfsHandle(const VfsHandle &) = delete;
  VfsHandle &operator=(const VfsHandle &) = delete;

  static Result init(void *object, const void *)
  {
    new (object) VfsHandle{};
    return E_OK;
  }

  static void deinit(void *object)
  {
    static_cast<VfsHandle *>(object)->~VfsHandle();
  }

  static void *root(void *object)
  {
    return static_cast<VfsHandle *>(object)->rootImpl();
  }

  static Result sync(void *object)
  {
    return static_cast<VfsHandle *>(object)->syncImpl();
  }

  void freeNodeProxy(VfsNodeProxy *proxy)
  {
    ::deinit(proxy);
  }

  VfsNodeProxy *makeNodeProxy(VfsNode *node)
  {
    const VfsNodeProxy::Config config{node};
    return static_cast<VfsNodeProxy *>(::init(VfsNodeProxyClass, &config));
  }

  void lock()
  {
    // TODO Mutex
  }

  void unlock()
  {
    // TODO Mutex
  }

protected:
  FsHandle m_base;
  VfsDirectory m_root;

  VfsHandle() :
    m_root{}
  {
    m_root.enter(this, nullptr);
  }

  void *rootImpl()
  {
    return makeNodeProxy(&m_root);
  }

  Result syncImpl()
  {
    // TODO
    return E_OK;
  }
};

#endif // VFS_SHELL_CORE_VFS_VFSHANDLE_HPP_
