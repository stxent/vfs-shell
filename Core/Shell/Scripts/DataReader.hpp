/*
 * Core/Shell/Scripts/DataReader.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_DATAREADER_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_DATAREADER_HPP_

#include <functional>
#include "Shell/ShellScript.hpp"

class DataReader: public ShellScript
{
public:
  DataReader(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    ShellScript{parent, firstArgument, lastArgument}
  {
  }

protected:
  Result read(void *buffer, FsNode *src, size_t blockSize, size_t blockCount, size_t skip,
      std::function<Result (const void *, size_t)> callback)
  {
    FsLength srcPosition = static_cast<FsLength>(blockSize) * skip;
    size_t blocks = 0;
    Result res = E_OK;

    // Copy file content
    while (!blockCount || blocks++ < blockCount)
    {
      size_t bytesRead;

      if ((res = fsNodeRead(src, FS_NODE_DATA, srcPosition, buffer, blockSize, &bytesRead)) == E_EMPTY)
      {
        res = E_OK;
        break;
      }

      if (res == E_OK)
      {
        if (bytesRead > 0)
        {
          srcPosition += bytesRead;

          if ((res = callback(buffer, bytesRead)) != E_OK)
            break;
        }
        else
          break;
      }
      else
      {
        tty() << "read error at " << srcPosition << Terminal::EOL;
        break;
      }
    }

    return res;
  }
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_DATAREADER_HPP_
