/*
 * Core/Vfs/VfsDataNode.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_VFS_VFSDATANODE_HPP_
#define VFS_SHELL_CORE_VFS_VFSDATANODE_HPP_

#include "Vfs/Vfs.hpp"
#include <memory>

class VfsDataNode: public VfsNode
{
public:
  VfsDataNode(const char *, const void *, size_t, time64_t = 0, FsAccess = FS_ACCESS_READ | FS_ACCESS_WRITE);

  virtual Result length(FsFieldType, FsLength *) override;
  virtual Result read(FsFieldType, FsLength, void *, size_t, size_t *) override;
  virtual Result write(FsFieldType, FsLength, const void *, size_t, size_t *) override;

private:
  static constexpr size_t INITIAL_LENGTH{16};

  size_t m_dataCapacity;
  size_t m_dataLength;
  std::unique_ptr<uint8_t []> m_dataBuffer;

  bool reallocateDataBuffer(size_t);
  Result writeDataBuffer(FsLength, const void *, size_t, size_t *);
};

#endif // VFS_SHELL_CORE_VFS_VFSDATANODE_HPP_
