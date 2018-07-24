/*
 * Shell.cpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <iterator>
#include "Shell/ArgParser.hpp"
#include "Shell/Evaluator.hpp"
#include "Shell/Scripts/Shell.hpp"
#include "Shell/ShellHelpers.hpp"

Shell::Shell(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
  ShellScript{parent, firstArgument, lastArgument},
  m_executable{extractExecutablePath(firstArgument, lastArgument)},
  m_terminal{this, parent->tty(), m_executable},
  m_semaphore{0},
  m_state{State::IDLE}
{
}

Result Shell::onEventReceived(const ScriptEvent *event)
{
  switch (event->event)
  {
    case ScriptEvent::Event::SERIAL_INPUT:
      if (m_state == State::IDLE)
      {
        m_semaphore.post();
      }
      else
      {
        m_terminal.onEventReceived(event);
      }
      break;

    default:
      break;
  }

  return E_OK;
}

Result Shell::run()
{
  auto &workDir = env()["PWD"];
  size_t position = 0;
  char command[COMMAND_BUFFER];
  char carriage = 0;
  bool interactive = m_executable == nullptr;
  bool echo = (interactive && strcmp(env()["ECHO"], "0") != 0) || strcmp(env()["DEBUG"], "1") == 0;

  if (interactive)
    m_terminal << workDir << "> ";

  do
  {
    if (interactive)
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

            if (interactive)
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
  while (interactive && m_state != State::STOP); // FIXME Rewrite slave logic more thoughtfully

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

const char *Shell::extractExecutablePath(ArgumentIterator firstArgument, ArgumentIterator lastArgument)
{
  struct Arguments
  {
    Arguments() :
      path{nullptr}
    {
    }

    const char *path;
  };

  static const ArgParser::Descriptor descriptors[] = {
      {nullptr, "FILE", "read commands from the file system entry", 1, positionalArgumentParser}
  };

  return ArgParser::parse<Arguments>(firstArgument, lastArgument,
      std::cbegin(descriptors), std::cend(descriptors)).path;
}

void Shell::positionalArgumentParser(void *object, const char *argument)
{
  *static_cast<const char **>(object) = argument;
}
