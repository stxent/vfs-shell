/*
 * Core/Shell/Scripts/ChecksumCrc32Script.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_CHECKSUMCRC32SCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_CHECKSUMCRC32SCRIPT_HPP_

#include <xcore/crc/crc32.h>
#include "Shell/ArgParser.hpp"
#include "Shell/Scripts/DataReader.hpp"
#include "Wrappers/Semaphore.hpp"

template<size_t BUFFER_SIZE>
class ChecksumCrc32Script: public DataReader
{
public:
  ChecksumCrc32Script(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    DataReader{parent, firstArgument, lastArgument},
    m_result{E_OK},
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

  virtual Result run() override
  {
    static const ArgParser::Descriptor descriptors[] = {
        {"--help", nullptr, "show this help message and exit", 0, Arguments::helpSetter},
        {nullptr, "FILE", "compute checksum for FILE", 0, nullptr}
    };

    const Arguments arguments = ArgParser::parse<Arguments>(m_firstArgument, m_lastArgument,
        std::cbegin(descriptors), std::cend(descriptors));

    if (arguments.help)
    {
      ArgParser::help(tty(), name(), std::cbegin(descriptors), std::cend(descriptors));
      return E_OK;
    }
    else
    {
      ArgParser::invoke(m_firstArgument, m_lastArgument, std::cbegin(descriptors), std::cend(descriptors),
          std::bind(&ChecksumCrc32Script::computeChecksum, this, std::placeholders::_1));
      return m_result;
    }
  }

  static const char *name()
  {
    return "cksum";
  }

private:
  static constexpr uint32_t INITIAL_CHECKSUM = 0;

  struct Arguments
  {
    Arguments() :
      help{false}
    {
    }

    bool help;

    static void helpSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->help = true;
    }
  };

  Result m_result;
  Semaphore m_semaphore;

  Result onDataRead(uint32_t *checksum, const void *buffer, size_t bytesRead)
  {
    while (m_semaphore.tryWait())
    {
      if (checkForTerminateRequest())
        return E_TIMEOUT;
    }

    *checksum = crc32Update(*checksum, buffer, bytesRead);
    return E_OK;
  }

  void computeChecksum(const char *positionalArgument)
  {
    if (m_result != E_OK)
      return;

    // Open the source node
    FsNode * const src = ShellHelpers::openSource(fs(), env(), positionalArgument);

    if (src != nullptr)
    {
      uint8_t buffer[BUFFER_SIZE];
      uint32_t checksum = INITIAL_CHECKSUM;

      using namespace std::placeholders;
      m_result = read(buffer, src, BUFFER_SIZE, 0, 0,
          std::bind(&ChecksumCrc32Script::onDataRead, this, &checksum, _1, _2));
      fsNodeFree(src);

      if (m_result == E_OK)
      {
        const auto format = tty().format();
        const auto width = tty().width();

        tty() << Terminal::Width{8} << Terminal::Format::HEX;
        tty() << checksum << "  " << positionalArgument << Terminal::EOL;
        tty() << format << width;
      }
    }
    else
    {
      tty() << name() << ": " << positionalArgument << ": open failed" << Terminal::EOL;
      m_result = E_ENTRY;
    }
  }

  bool checkForTerminateRequest()
  {
    static constexpr size_t RX_BUFFER = 16;
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

    return false;
  }
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_CHECKSUMCRC32SCRIPT_HPP_
