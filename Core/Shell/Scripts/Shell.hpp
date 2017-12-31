/*
 * Core/Shell/Scripts/Shell.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_SHELL_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_SHELL_HPP_

#include <atomic>
#include <iterator>
#include "Shell/ShellHelpers.hpp"
#include "Shell/ShellScript.hpp"
#include "Shell/TerminalProxy.hpp"
#include "Wrappers/Semaphore.hpp"

class Shell: public ShellScript
{
  enum class State
  {
    IDLE,
    EXEC,
    STOP
  };

public:
  Shell(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    ShellScript{parent, firstArgument, lastArgument},
    m_executable{extractExecutablePath(firstArgument, lastArgument)},
    m_terminal{this, parent->tty(), m_executable},
    m_semaphore{0},
    m_state{State::IDLE}
  {
  }

  static const char *name()
  {
    return "sh";
  }

  virtual Result onEventReceived(const ScriptEvent *event) override
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

  virtual Terminal &tty() override
  {
    return m_terminal;
  }

  virtual Result run() override;

private:
  static constexpr size_t ARGUMENT_COUNT  = 16;
  static constexpr size_t COMMAND_BUFFER  = 256;
  static constexpr size_t RX_BUFFER       = 64;

  static const char *extractExecutablePath(ArgumentIterator firstArgument, ArgumentIterator lastArgument)
  {
    const char *path = nullptr;
    const ArgParser::Descriptor descriptors[] = {
        {nullptr, nullptr, 1, positionalArgumentParser, &path}
    };
    ArgParser::parse(firstArgument, lastArgument, std::begin(descriptors), std::end(descriptors));
    return path;
  }

  static void positionalArgumentParser(void *object, const char *argument)
  {
    *static_cast<const char **>(object) = argument;
  }

  const char *m_executable;
  TerminalProxy m_terminal;
  Semaphore m_semaphore;
  std::atomic<State> m_state;

  void evaluate(char *, size_t);
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_SHELL_HPP_
