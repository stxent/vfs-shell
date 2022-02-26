/*
 * LimitedTestNode.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "LimitedTestNode.hpp"

LimitedTestNode::LimitedTestNode(size_t limit) :
  VfsDataNode{},
  m_limit{limit}
{
}

Result LimitedTestNode::read(FsFieldType type, FsLength position, void *buffer, size_t length, size_t *read)
{
  return VfsDataNode::read(type, position, buffer, length, read);
}

Result LimitedTestNode::write(FsFieldType type, FsLength position, const void *buffer, size_t length, size_t *written)
{
  size_t left = length;

  if (type == FS_NODE_DATA)
    left = std::min(length, m_limit - static_cast<size_t>(position));

  return VfsDataNode::write(type, position, buffer, left, written);
}
