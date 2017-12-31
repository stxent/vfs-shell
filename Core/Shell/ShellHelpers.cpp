/*
 * Shell.cpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Shell/Settings.hpp"
#include "Shell/ShellHelpers.hpp"

const std::array<const char *, E_RESULT_END> ShellHelpers::PRINTABLE_RESULTS = {
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

const char *ShellHelpers::extractName(const char *path)
{
  ssize_t length = 0;

  for (ssize_t pos = strlen(path) - 1; pos >= 0; --pos, ++length)
  {
    if (path[pos] == '/')
      return length ? path + pos + 1 : nullptr;
  }

  return length ? path : nullptr;
}

const char *ShellHelpers::followNextPart(FsHandle *handle, FsNode **node, const char *path, bool leaf)
{
  char nextPart[Settings::ENTRY_NAME_LENGTH];

  path = getChunk(nextPart, path);

  if (!strlen(nextPart))
  {
    path = nullptr;
  }
  else if (!strcmp(nextPart, ".") || !strcmp(nextPart, ".."))
  {
    // Path contains forbidden directories
    path = nullptr;
  }
  else if (*node == nullptr)
  {
    if (nextPart[0] == '/')
    {
      *node = static_cast<FsNode *>(fsHandleRoot(handle));

      if (*node == nullptr)
        path = nullptr;
    }
    else
    {
      path = nullptr;
    }
  }
  else if (leaf || strlen(path))
  {
    FsNode *child = static_cast<FsNode *>(fsNodeHead(*node));
    fsNodeFree(*node);

    while (child)
    {
      char nodeName[Settings::ENTRY_NAME_LENGTH];
      const Result res = fsNodeRead(child, FS_NODE_NAME, 0, nodeName, sizeof(nodeName), nullptr);

      if (res == E_OK)
      {
        if (!strcmp(nextPart, nodeName))
          break;
      }

      if (res != E_OK || fsNodeNext(child) != E_OK)
      {
        fsNodeFree(child);
        child = nullptr;
        break;
      }
    }

    // Check whether the node is found
    if (child != nullptr)
      *node = child;
    else
      path = nullptr;
  }

  return path;
}

FsNode *ShellHelpers::followPath(FsHandle *handle, const char *path, bool leaf)
{
  FsNode *node = nullptr;

  while (path && *path)
    path = followNextPart(handle, &node, path, leaf);

  return path != nullptr ? node : nullptr;
}

const char *ShellHelpers::getChunk(char *dst, const char *src)
{
  // Output buffer length should be greater or equal to maximum name length
  size_t counter = 0;

  if (!*src)
    return src;

  if (*src == '/')
  {
    *dst++ = '/';
    *dst = '\0';
    return src + 1;
  }

  while (*src && counter++ < Settings::ENTRY_NAME_LENGTH - 1)
  {
    if (*src == '/')
    {
      ++src;
      break;
    }
    *dst++ = *src++;
  }
  *dst = '\0';

  return src;
}

void ShellHelpers::joinPaths(char *buffer, const char *directory, const char *path)
{
  if (!path || !strlen(path))
  {
    strcpy(buffer, directory);
  }
  else if (path[0] != '/')
  {
    const size_t directoryLength = strlen(directory);

    strcpy(buffer, directory);
    if (directoryLength > 1 && directory[directoryLength - 1] != '/')
      strcat(buffer, "/");
    strcat(buffer, path);
  }
  else
    strcpy(buffer, path);
}

bool ShellHelpers::stripName(char *buffer)
{
  const char * const position = ShellHelpers::extractName(buffer);

  if (position != nullptr)
  {
    size_t offset = position - buffer;

    if (offset > 1)
      --offset;
    buffer[offset] = '\0';

    return true;
  }
  else
    return false;
}

FsNode *ShellHelpers::openBaseNode(FsHandle *handle, const char *path)
{
  return followPath(handle, path, false);
}

FsNode *ShellHelpers::openNode(FsHandle *handle, const char *path)
{
  return followPath(handle, path, true);
}

FsNode *ShellHelpers::openScript(FsHandle *handle, Environment &env, const char *path)
{
  char absolutePath[Settings::PWD_LENGTH];
  ShellHelpers::joinPaths(absolutePath, env["PATH"], path);
  return ShellHelpers::openNode(handle, absolutePath);
}

FsNode *ShellHelpers::openSink(FsHandle *fs, Environment &env, TimeProvider &time, const char *path, bool overwrite,
    Result *invocationResult)
{
  char absolutePath[Settings::PWD_LENGTH];
  FsNode *node = nullptr;
  Result res = E_OK;

  ShellHelpers::joinPaths(absolutePath, env["PWD"], path);

  // Check node existence
  FsNode * const existingNode = ShellHelpers::openNode(fs, absolutePath);
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
    if (res == E_OK)
    {
      if ((root = ShellHelpers::openBaseNode(fs, absolutePath)) == nullptr)
        res = E_ENTRY;
    }

    // Create new node
    if (res == E_OK)
    {
      const char * const nodeName = ShellHelpers::extractName(absolutePath);
      const auto nodeTime = time.microtime();
      const FsFieldDescriptor descriptors[] = {
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

      res = fsNodeCreate(root, descriptors, ARRAY_SIZE(descriptors));
      fsNodeFree(root);
    }
  }

  // Open created node
  if (res == E_OK && node == nullptr)
  {
    if ((node = ShellHelpers::openNode(fs, absolutePath)) == nullptr)
      res = E_ENTRY;
  }

  if (invocationResult != nullptr)
    *invocationResult = res;
  return node;
}

FsNode *ShellHelpers::openSource(FsHandle *fs, Environment &env, const char *path)
{
  char absolutePath[Settings::PWD_LENGTH];

  ShellHelpers::joinPaths(absolutePath, env["PWD"], path);
  return ShellHelpers::openNode(fs, absolutePath);
}
