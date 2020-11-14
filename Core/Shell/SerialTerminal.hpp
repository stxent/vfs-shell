/*
 * Core/Shell/SerialTerminal.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SERIALTERMINAL_HPP_
#define VFS_SHELL_CORE_SHELL_SERIALTERMINAL_HPP_

#include "Shell/Terminal.hpp"
#include <xcore/interface.h>
#include <list>

class Script;

class SerialTerminal: public Terminal
{
public:
  SerialTerminal(Interface *, bool = false);
  virtual ~SerialTerminal();

  virtual void subscribe(Script *) override;
  virtual void unsubscribe(const Script *) override;
  virtual size_t read(char *, size_t) override;
  virtual size_t write(const char *, size_t) override;

private:
  static constexpr size_t BUFFER_SIZE{64};

  Interface *m_interface;
  std::list<Script *> m_subscribers;

  void dataCallback();

  static void dataCallbackHelper(void *argument)
  {
    static_cast<SerialTerminal *>(argument)->dataCallback();
  }
};

#endif // VFS_SHELL_CORE_SHELL_SERIALTERMINAL_HPP_
