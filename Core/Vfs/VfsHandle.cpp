/*
 * VfsHandle.cpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Vfs/VfsHandle.hpp"

const FsHandleClass VfsHandle::table{
    sizeof(VfsHandle), // size
    VfsHandle::init,   // init
    VfsHandle::deinit, // deinit

    VfsHandle::root,   // root
    VfsHandle::sync    // sync
};
const FsHandleClass * const VfsHandleClass = &VfsHandle::table;
