/*
 * Core/Shell/Scripts/PrintRawDataScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_PRINTRAWDATASCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_PRINTRAWDATASCRIPT_HPP_

#include "Shell/ArgParser.hpp"
#include "Shell/Scripts/DataReader.hpp"

template<size_t BUFFER_SIZE>
class PrintRawDataScript: public DataReader
{
public:
  PrintRawDataScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    DataReader{parent, firstArgument, lastArgument},
    m_result{E_OK}
  {
  }

  virtual Result run() override
  {
    static const ArgParser::Descriptor descriptors[] = {
        {"--help", nullptr, "show this help message and exit", 0, Arguments::helpSetter},
        {nullptr, "FILE", "display content of FILE", 0, nullptr}
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
          [this](const char *key){ displayData(key); });
      return m_result;
    }
  }

  static const char *name()
  {
    return "cat";
  }

private:
  static constexpr uint32_t INITIAL_CHECKSUM{0};

  struct Arguments
  {
    bool help{false};

    static void helpSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->help = true;
    }
  };

  Result m_result;

  Result onDataRead(const void *buffer, size_t bytesRead)
  {
    const char *position = static_cast<const char *>(buffer);
    size_t bytesLeft = bytesRead;

    while (bytesLeft)
    {
      const size_t bytesWritten = tty().write(position, bytesLeft);
      bytesLeft -= bytesWritten;
      position += bytesWritten;
    }

    return E_OK;
  }

  void displayData(const char *positionalArgument)
  {
    // Open the source node
    FsNode * const src = ShellHelpers::openSource(fs(), env(), positionalArgument);

    if (src != nullptr)
    {
      uint8_t buffer[BUFFER_SIZE];

      m_result = read(buffer, src, BUFFER_SIZE, 0, 0, std::bind(&PrintRawDataScript::onDataRead, this,
          std::placeholders::_1, std::placeholders::_2));
      fsNodeFree(src);
    }
    else
    {
      tty() << name() << ": " << positionalArgument << ": open failed" << Terminal::EOL;
      m_result = E_ENTRY;
    }
  }
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_PRINTRAWDATASCRIPT_HPP_
