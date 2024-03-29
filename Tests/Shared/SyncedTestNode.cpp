/*
 * SyncedTestNode.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "SyncedTestNode.hpp"

SyncedTestNode::SyncedTestNode(std::function<Result ()> callback) :
  VfsDataNode{},
  m_sem{0},
  m_callback{callback}
{
}

void SyncedTestNode::post()
{
  m_sem.post();
}

Result SyncedTestNode::read(FsFieldType type, FsLength position, void *buffer, size_t length, size_t *read)
{
  Result res = E_OK;

  if (type == FS_NODE_DATA)
  {
    if (m_callback)
      res = m_callback();
    m_sem.wait();
  }

  if (res != E_OK)
    return res;

  return VfsDataNode::read(type, position, buffer, length, read);
}
