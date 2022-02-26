/*
 * Shell.cpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Shell/Settings.hpp"
#include "Shell/ShellHelpers.hpp"
#include "Vfs/Vfs.hpp"
#include <xcore/fs/utils.h>

Terminal &operator<<(Terminal &output, ShellHelpers::ResultSerializer container)
{
  static const std::array<const char *, E_RESULT_END> PRINTABLE_RESULTS{
      "E_OK",
      "E_ERROR",
      "E_MEMORY",
      "E_ACCESS",
      "E_ADDRESS",
      "E_BUSY",
      "E_DEVICE",
      "E_IDLE",
      "E_INTERFACE",
      "E_INVALID",
      "E_TIMEOUT",
      "E_VALUE",
      "E_ENTRY",
      "E_EXIST",
      "E_EMPTY",
      "E_FULL"
  };

  if (container.m_result < PRINTABLE_RESULTS.size())
    output << PRINTABLE_RESULTS[container.m_result];
  else
    output << container.m_result;

  return output;
}

Result ShellHelpers::injectNode(FsHandle *handle, VfsNode *node, const char *path)
{
  if (node == nullptr)
    return E_VALUE;

  const char * const name = fsExtractName(path);

  if (!name)
    return E_VALUE;

  if (!node->rename(name))
    return E_MEMORY;

  // Open base node
  FsNode * const parent = fsOpenBaseNode(handle, path);

  if (!parent)
    return E_ENTRY;

  // Inject new node
  const std::array<FsFieldDescriptor, 1> entryFields = {{
      {&node, sizeof(node), static_cast<FsFieldType>(VfsNode::VFS_NODE_OBJECT)}
  }};

  const auto res = fsNodeCreate(parent, entryFields.data(), entryFields.size());
  fsNodeFree(parent);

  return res;
}

FsNode *ShellHelpers::openScript(FsHandle *handle, Environment &env, const char *path)
{
  char absolutePath[Settings::PWD_LENGTH];
  FsNode *node;

  // Search node in the PATH
  fsJoinPaths(absolutePath, env["PATH"], path);
  if ((node = fsOpenNode(handle, absolutePath)) != nullptr)
    return node;

  // Search node in the current directory
  fsJoinPaths(absolutePath, env["PWD"], path);
  if ((node = fsOpenNode(handle, absolutePath)) != nullptr)
    return node;

  return nullptr;
}

FsNode *ShellHelpers::openSink(FsHandle *fs, Environment &env, TimeProvider &time, const char *path, bool overwrite,
    Result *invocationResult)
{
  char absolutePath[Settings::PWD_LENGTH];
  FsNode *node = nullptr;
  Result res = E_OK;

  fsJoinPaths(absolutePath, env["PWD"], path);

  // Check node existence
  FsNode * const existingNode = fsOpenNode(fs, absolutePath);
  if (existingNode != nullptr)
  {
    if (overwrite)
    {
      node = existingNode;
    }
    else
    {
      fsNodeFree(existingNode);
      res = E_EXIST;
    }
  }
  else
  {
    FsNode *root = nullptr;

    // Open directory
    if ((root = fsOpenBaseNode(fs, absolutePath)) == nullptr)
      res = E_ENTRY;

    // Create new node
    if (res == E_OK)
    {
      const char * const nodeName = fsExtractName(absolutePath);
      const auto nodeTime = time.getTime();
      const FsFieldDescriptor fields[] = {
          // Name descriptor
          {
              nodeName,
              strlen(nodeName) + 1,
              FS_NODE_NAME
          },
          // Access time descriptor
          {
              &nodeTime,
              sizeof(nodeTime),
              FS_NODE_TIME
          },
          // Data descriptor
          {
              nullptr,
              0,
              FS_NODE_DATA
          }
      };

      res = fsNodeCreate(root, fields, ARRAY_SIZE(fields));
      fsNodeFree(root);
    }
  }

  // Open created node
  if (res == E_OK && node == nullptr)
  {
    if ((node = fsOpenNode(fs, absolutePath)) == nullptr)
      res = E_ENTRY;
  }

  if (invocationResult != nullptr)
    *invocationResult = res;
  return node;
}

FsNode *ShellHelpers::openSource(FsHandle *fs, Environment &env, const char *path)
{
  char absolutePath[Settings::PWD_LENGTH];

  fsJoinPaths(absolutePath, env["PWD"], path);
  return fsOpenNode(fs, absolutePath);
}
