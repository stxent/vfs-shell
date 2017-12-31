/*
 * ShellScript.cpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <algorithm>
#include <cassert>
#include <cctype>
#include <iterator>
#include <xcore/bits.h>
#include "Shell/ArgParser.hpp"
#include "Shell/Evaluator.hpp"
#include "Shell/Scripts/Shell.hpp"

Result Shell::run()
{
  auto &workDir = env()["PWD"];
  size_t position = 0;
  char command[COMMAND_BUFFER];
  char carriage = 0;
  bool silent = m_executable != nullptr;
  bool echo = (!silent && strcmp(env()["ECHO"], "0") != 0) || strcmp(env()["DEBUG"], "0") == 0;

  if (!silent)
    m_terminal << workDir << "> ";

  do
  {
    if (!silent)
      m_semaphore.wait();

    char rxBuffer[RX_BUFFER];
    size_t rxCount;

    while (m_state != State::STOP && (rxCount = m_terminal.read(rxBuffer, sizeof(rxBuffer))))
    {
      const char *outputStart = rxBuffer;
      const char *outputEnd = rxBuffer;

      for (size_t i = 0; i < rxCount; ++i)
      {
        const char c = rxBuffer[i];

        if (c == '\x03') // End of text
        {
          m_state = State::STOP;

          if (echo)
          {
            m_terminal.write(outputStart, outputEnd - outputStart);
            m_terminal << "^C" << Terminal::EOL;
          }
          outputStart = outputEnd = rxBuffer + i + 1;

          break;
        }
        else if (c == '\r' || c == '\n') // New line
        {
          if (!carriage || carriage == c)
          {
            command[position] = '\0';

            if (echo)
            {
              m_terminal.write(outputStart, outputEnd - outputStart);
              m_terminal << Terminal::EOL;
            }
            outputStart = outputEnd = rxBuffer + i + 1;

            evaluate(command, position);
            position = 0;

            if (!silent)
              m_terminal << workDir << "> ";
          }
          carriage = c;
        }
        else if (c == '\b' || c == '\x7F') // Backspace
        {
          if (position > 0)
          {
            if (echo)
            {
              m_terminal.write(outputStart, outputEnd - outputStart);
              m_terminal << "\x08 \x08";
            }
            outputStart = outputEnd = rxBuffer + i + 1;

            --position;
          }
          carriage = 0;
        }
        else
        {
          if (position < sizeof(command) - 1)
          {
            command[position++] = rxBuffer[i];
            outputEnd = rxBuffer + i + 1;
          }
          carriage = 0;
        }
      }

      if (echo)
        m_terminal.write(outputStart, outputEnd - outputStart);
    }
  }
  while (!silent && m_state != State::STOP); // FIXME Rewrite slave logic more thoughtfully

  return E_OK; // TODO
}

void Shell::evaluate(char *command, size_t length)
{
  char *arguments[ARGUMENT_COUNT];
  size_t count;
  Result res;

  res = ShellHelpers::parseCommandString(std::begin(arguments), std::end(arguments), command, length, &count);
  if (res == E_OK)
  {
    if (arguments[0][0] != '#')
    {
      Evaluator<ArgumentIterator> evaluator{this, std::cbegin(arguments), std::cbegin(arguments) + count};

      m_state = State::EXEC;
      res = evaluator.run();
      if (m_state != State::STOP)
        m_state = State::IDLE;

      if (res != E_OK)
      {
        m_terminal << name() << ": command failed, error code ";
        if (res < ShellHelpers::PRINTABLE_RESULTS.size())
          m_terminal << ShellHelpers::PRINTABLE_RESULTS[res];
        else
          m_terminal << res;
        m_terminal << Terminal::EOL;
      }
    }
  }
  else if (res != E_EMPTY)
  {
    m_terminal << name() << ": incorrect command string" << Terminal::EOL;
  }
}
