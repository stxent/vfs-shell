/*
 * Tests/Shared/SyncedTestNode.hpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_TESTS_SHARED_SYNCEDTESTNODE_HPP_
#define VFS_SHELL_TESTS_SHARED_SYNCEDTESTNODE_HPP_

#include "Vfs/VfsDataNode.hpp"
#include "Wrappers/Semaphore.hpp"
#include <functional>

class SyncedTestNode: public VfsDataNode
{
public:
  SyncedTestNode(const char *, std::function<Result ()> = nullptr);

  virtual Result read(FsFieldType, FsLength, void *, size_t, size_t *) override;
  virtual Result write(FsFieldType, FsLength, const void *, size_t, size_t *) override;

  void post();

private:
  Os::Semaphore m_sem;
  std::function<Result ()> m_callback;
};

#endif // VFS_SHELL_TESTS_SHARED_SYNCEDTESTNODE_HPP_
