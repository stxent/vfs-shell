/*
 * Core/Shell/ShellHelpers.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SHELLHELPERS_HPP_
#define VFS_SHELL_CORE_SHELL_SHELLHELPERS_HPP_

#include "Shell/Script.hpp"
#include "Shell/ScriptHeaders.hpp"
#include <xcore/fs/fs.h>
#include <cctype>

class Terminal;

struct ShellHelpers
{
  struct ResultSerializer
  {
    Result m_result;
  };

  ShellHelpers() = delete;
  ShellHelpers(const ShellHelpers &) = delete;
  ShellHelpers &operator=(const ShellHelpers &) = delete;

  static FsNode *openScript(FsHandle *, Environment &, const char *);
  static FsNode *openSink(FsHandle *, Environment &, TimeProvider &, const char *, bool, Result *);
  static FsNode *openSource(FsHandle *, Environment &, const char *);

  template<typename T>
  static Result parseCommandString(T firstArgument, T lastArgument, char *input, size_t inputLength,
      size_t *argumentCount)
  {
    const T head = firstArgument;
    size_t start = 0;
    bool braces = false;
    bool spaces = true;

    while (inputLength > 0)
    {
      if (iscntrl(input[inputLength - 1]))
        --inputLength;
      else
        break;
    }

    for (size_t pos = 0; pos < inputLength; ++pos)
    {
      const char c = input[pos];

      if (!braces && (c == ' ' || c == '\t'))
      {
        if (!spaces)
        {
          spaces = true;
          *firstArgument++ = input + start;
          input[pos] = '\0';
        }
        continue;
      }

      if (firstArgument == lastArgument)
        return E_FULL;

      if (c == '"')
      {
        if (!braces)
        {
          braces = true;
          spaces = false;
          start = pos + 1;
          continue;
        }
        else
        {
          braces = false;
          spaces = true;
          *firstArgument++ = input + start;
          input[pos] = '\0';
          continue;
        }
      }

      if (spaces)
      {
        spaces = false;
        start = pos;
      }

      if (pos == inputLength - 1)
      {
        *firstArgument++ = input + start;
        input[pos + 1] = '\0';
      }
    }

    if (firstArgument != head && argumentCount != nullptr)
      *argumentCount = firstArgument - head;

    return firstArgument != head ? E_OK : E_EMPTY;
  }
};

Terminal &operator<<(Terminal &, ShellHelpers::ResultSerializer);

#endif // VFS_SHELL_CORE_SHELL_SHELLHELPERS_HPP_
