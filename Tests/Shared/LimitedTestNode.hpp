/*
 * Tests/Shared/LimitedTestNode.hpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_TESTS_SHARED_LIMITEDTESTNODE_HPP_
#define VFS_SHELL_TESTS_SHARED_LIMITEDTESTNODE_HPP_

#include "Vfs/VfsDataNode.hpp"

class LimitedTestNode: public VfsDataNode
{
public:
  LimitedTestNode(size_t);

  virtual Result read(FsFieldType, FsLength, void *, size_t, size_t *) override;
  virtual Result write(FsFieldType, FsLength, const void *, size_t, size_t *) override;

private:
  size_t m_limit;
};

#endif // VFS_SHELL_TESTS_SHARED_LIMITEDTESTNODE_HPP_
