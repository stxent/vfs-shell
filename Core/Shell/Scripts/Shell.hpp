/*
 * Core/Shell/Scripts/Shell.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_SHELL_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_SHELL_HPP_

#include <atomic>
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
  Shell(Script *, ArgumentIterator, ArgumentIterator);

  virtual Result onEventReceived(const ScriptEvent *) override;
  virtual Result run() override;

  static const char *name()
  {
    return "sh";
  }

  virtual Terminal &tty() override
  {
    return m_terminal;
  }

private:
  static constexpr size_t ARGUMENT_COUNT  = 16;
  static constexpr size_t COMMAND_BUFFER  = 256;
  static constexpr size_t RX_BUFFER       = 64;

  static const char *extractExecutablePath(ArgumentIterator, ArgumentIterator);
  static void positionalArgumentParser(void *, const char *);

  const char *m_executable;
  TerminalProxy m_terminal;
  Semaphore m_semaphore;
  std::atomic<State> m_state;

  void evaluate(char *, size_t);
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_SHELL_HPP_
