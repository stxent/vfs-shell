/*
 * Core/Shell/Scripts/PrintDataScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_PRINTDATASCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_PRINTDATASCRIPT_HPP_

#include "Shell/ArgParser.hpp"
#include "Shell/Scripts/DataReader.hpp"

template<size_t BUFFER_SIZE>
class PrintDataScript: public DataReader
{
public:
  PrintDataScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
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
          std::bind(&PrintDataScript::printData, this, std::placeholders::_1));
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
      showHelpMessage{false}
    {
    }

    bool showHelpMessage;
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

  void printData(const char *positionalArgument)
  {
    // Open the source node
    FsNode * const src = ShellHelpers::openSource(fs(), env(), positionalArgument);

    if (src != nullptr)
    {
      uint8_t buffer[BUFFER_SIZE];

      using namespace std::placeholders;
      m_result = read(buffer, src, BUFFER_SIZE, 0, 0, std::bind(&PrintDataScript::onDataRead, this, _1, _2));
      fsNodeFree(src);
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

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_PRINTDATASCRIPT_HPP_
