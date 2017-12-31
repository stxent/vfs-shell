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

template<size_t BUFFER_SIZE>
class ChecksumCrc32Script: public DataReader
{
public:
  ChecksumCrc32Script(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    DataReader{parent, firstArgument, lastArgument},
    m_result{E_OK}
  {
  }

  virtual Result run() override
  {
    Arguments arguments;
    const std::array<ArgParser::Descriptor, 1> descriptors = {
        {
            {"--help", "print help message", 0, boolArgumentSetter, &arguments.showHelpMessage}
        }
    };

    ArgParser::parse(m_firstArgument, m_lastArgument, descriptors.begin(), descriptors.end());

    if (arguments.showHelpMessage)
    {
      ArgParser::help(tty(), descriptors.begin(), descriptors.end());
      return E_OK;
    }
    else
    {
      ArgParser::invoke(m_firstArgument, m_lastArgument, descriptors.begin(), descriptors.end(),
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
      showHelpMessage{false}
    {
    }

    bool showHelpMessage;
  };

  Result m_result;

  Result onDataRead(uint32_t *checksum, const void *buffer, size_t bytesRead)
  {
    *checksum = crc32Update(*checksum, buffer, bytesRead);
    return E_OK;
  }

  void computeChecksum(const char *positionalArgument)
  {
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
        tty() << Terminal::Width{8} << Terminal::Format::HEX;
        tty() << checksum << "  " << positionalArgument << Terminal::EOL;
        tty() << Terminal::Format::DEC << Terminal::Width{1};
      }
    }
    else
    {
      tty() << name() << ": " << positionalArgument << ": open failed" << Terminal::EOL;
      m_result = E_ENTRY;
    }
  }

  static void boolArgumentSetter(void *object, const char *)
  {
    *static_cast<bool *>(object) = true;
  }
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_CHECKSUMCRC32SCRIPT_HPP_
