/*
 * Core/Shell/Scripts/DataReader.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_DATAREADER_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_DATAREADER_HPP_

#include <functional>
#include "Shell/ShellScript.hpp"
#include "Wrappers/Semaphore.hpp"

class DataReader: public ShellScript
{
public:
  DataReader(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    ShellScript{parent, firstArgument, lastArgument},
    m_semaphore{0}
  {
  }

  virtual Result onEventReceived(const ScriptEvent *event) override
  {
    switch (event->event)
    {
      case ScriptEvent::Event::SERIAL_INPUT:
        if (m_semaphore.value() <= 0)
          m_semaphore.post();
        break;

      default:
        break;
    }

    return E_OK;
  }

protected:
  Semaphore m_semaphore;

  bool isTerminateRequested()
  {
    static constexpr size_t RX_BUFFER = 16;

    while (m_semaphore.tryWait())
    {
      char rxBuffer[RX_BUFFER];
      size_t rxCount;

      while ((rxCount = tty().read(rxBuffer, sizeof(rxBuffer))))
      {
        for (size_t i = 0; i < rxCount; ++i)
        {
          if (rxBuffer[i] == '\x03') // End of text
            return true;
        }
      }
    }

    return false;
  }

  Result read(void *buffer, FsNode *src, size_t blockSize, size_t blockCount, size_t skip,
      std::function<Result (const void *, size_t)> callback)
  {
    FsLength srcPosition = static_cast<FsLength>(blockSize) * skip;
    size_t blocks = 0;
    Result res = E_OK;

    // Copy file content
    while (!blockCount || blocks++ < blockCount)
    {
      if (isTerminateRequested())
      {
        res = E_TIMEOUT;
        break;
      }

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
