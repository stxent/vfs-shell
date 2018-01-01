/*
 * Core/Shell/Scripts/PrintHexDataScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_PRINTHEXDATASCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_PRINTHEXDATASCRIPT_HPP_

#include "Shell/ArgParser.hpp"
#include "Shell/Scripts/DataReader.hpp"

template<size_t BUFFER_SIZE>
class PrintHexDataScript: public DataReader
{
public:
  PrintHexDataScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    DataReader{parent, firstArgument, lastArgument},
    m_result{E_OK}
  {
  }

  virtual Result run() override
  {
    static const ArgParser::Descriptor descriptors[] = {
        {"--help", nullptr, "show this help message and exit", 0, Arguments::helpSetter},
        {nullptr, "FILE", "display content of FILE in hexadecimal", 0, nullptr}
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
          std::bind(&PrintHexDataScript::displayData, this, std::placeholders::_1));
      return m_result;
    }
  }

  static const char *name()
  {
    return "hexdump";
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

  Result onDataRead(const void *buffer, size_t bytesRead)
  {
    static constexpr size_t ROW_SIZE = 16;
    auto hexify = [](uint8_t value) { return value < 10 ? '0' + value : 'a' + (value - 10); };

    for (size_t row = 0; row < (bytesRead + (ROW_SIZE - 1)) / ROW_SIZE; ++row)
    {
      const size_t currentRowSize = std::min(bytesRead - row * ROW_SIZE, ROW_SIZE);
      char str[ROW_SIZE * 3];
      char *pos = str;

      for (size_t col = 0; col < currentRowSize; ++col)
      {
        const uint8_t val = *(static_cast<const uint8_t *>(buffer) + row * ROW_SIZE + col);

        *pos++ = hexify(val >> 4);
        *pos++ = hexify(val & 0x0F);
        *pos++ = col + 1 != currentRowSize ? ' ' : '\0';
      }

      tty() << str << Terminal::EOL;
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

      m_result = read(buffer, src, BUFFER_SIZE, 0, 0, std::bind(&PrintHexDataScript::onDataRead, this,
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

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_PRINTHEXDATASCRIPT_HPP_
