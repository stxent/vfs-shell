/*
 * Shell.cpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Shell/ArgParser.hpp"
#include "Shell/Evaluator.hpp"
#include "Shell/Scripts/Shell.hpp"
#include "Shell/ShellHelpers.hpp"
#include <iterator>

Shell::Shell(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
  ShellScript{parent, firstArgument, lastArgument},
  m_executable{extractExecutablePath(firstArgument, lastArgument)},
  m_terminal{this, parent->tty(), m_executable},
  m_semaphore{m_executable != nullptr ? 1 : 0},
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

    case ScriptEvent::Event::SIGNAL_RAISED:
      m_state = State::STOP;
      break;

    default:
      break;
  }

  return E_OK;
}

Result Shell::run()
{
  if (!m_terminal.isInputReady())
  {
    // Input script not found or could not be opened
    return E_ENTRY;
  }

  const bool interactive = m_executable == nullptr;
  const bool echo = (interactive && strcmp(env()["ECHO"], "0") != 0) || strcmp(env()["DEBUG"], "0") != 0;

  EscapeSeqParser escapeParser;
  LineParser lineParser{m_terminal, echo};

  auto &pwd = env()["PWD"];
  Result lastCommandResult = E_OK;

  if (interactive)
    showPrompt(pwd);

  do
  {
    m_semaphore.wait();

    char rxBuffer[RX_BUFFER];
    size_t rxCount;

    while (m_state != State::STOP && (rxCount = m_terminal.read(rxBuffer, sizeof(rxBuffer))))
    {
      for (size_t i = 0; i < rxCount; ++i)
      {
        const auto escapeStatus = escapeParser.parse(rxBuffer[i]);

        if (escapeStatus == EscapeSeqParser::Status::COMPLETED)
        {
          const auto ev = escapeParser.event();

          if (ev == EscapeSeqParser::RIGHT)
            lineParser.moveCursorRight();
          else if (ev == EscapeSeqParser::LEFT)
            lineParser.moveCursorLeft();
          else if (ev == EscapeSeqParser::DEL)
            lineParser.eraseNext();
        }
        else if (escapeStatus == EscapeSeqParser::Status::DISCARDED)
        {
          const auto lineStatus = lineParser.parse(rxBuffer[i]);
          bool promptRequired = false;

          if (lineStatus == LineParser::Status::COMPLETED || lineStatus == LineParser::Status::TERMINATED)
          {
            // Data buffered in the parser will be modified during evaluation to preserve memory
            lastCommandResult = evaluate(lineParser.data(), lineParser.length(), echo);
            lineParser.reset();

            if (lineStatus != LineParser::Status::TERMINATED)
              promptRequired = m_state != State::STOP;
            else
              m_state = State::STOP;
          }

          if (m_state == State::STOP)
            break;

          if (promptRequired && interactive)
            showPrompt(pwd);
        }
      }
    }
  }
  while (m_state != State::STOP);

  return lastCommandResult;
}

Result Shell::evaluate(char *command, size_t length, bool echo)
{
  char *arguments[ARGUMENT_COUNT];
  size_t count;
  Result res;

  res = ShellHelpers::parseCommandString(std::begin(arguments), std::end(arguments), command, length, &count);
  if (res == E_OK)
  {
    if (arguments[0][0] != '#')
    {
      char text[16];
      Evaluator<ArgumentIterator> evaluator{this, std::cbegin(arguments), std::cbegin(arguments) + count};

      m_state = State::EXEC;
      res = evaluator.run();
      if (m_state != State::STOP)
        m_state = State::IDLE;

      // Save result value
      TerminalHelpers::int2str<int>(text, static_cast<int>(res));
      env()["?"] = text;

      if (res != E_OK && echo)
        m_terminal << name() << ": command error " << ShellHelpers::ResultSerializer{res} << Terminal::EOL;
    }
  }
  else if (res != E_EMPTY && echo)
  {
    m_terminal << name() << ": incorrect input" << Terminal::EOL;
  }

  return res;
}

void Shell::showPrompt(EnvironmentVariable &pwd)
{
  m_terminal << pwd << "> ";
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
