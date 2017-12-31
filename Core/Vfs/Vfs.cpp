/*
 * Vfs.cpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Vfs/Vfs.hpp"
#include "Vfs/VfsHandle.hpp"

static const FsNodeClass vfsNodeProxyTable = {
    sizeof(struct VfsNodeProxy), // size
    VfsNodeProxy::init,          // init
    VfsNodeProxy::deinit,        // deinit

    VfsNodeProxy::create,        // create
    VfsNodeProxy::head,          // head
    VfsNodeProxy::free,          // free
    VfsNodeProxy::length,        // length
    VfsNodeProxy::next,          // next
    VfsNodeProxy::read,          // read
    VfsNodeProxy::remove,        // remove
    VfsNodeProxy::write          // write
};

const FsNodeClass * const VfsNodeProxyClass = &vfsNodeProxyTable;

void VfsNodeProxy::freeImpl()
{
  return m_node->m_handle->freeNodeProxy(this);
}
