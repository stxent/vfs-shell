/*
 * Core/Shell/SerialTerminal.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SERIALTERMINAL_HPP_
#define VFS_SHELL_CORE_SHELL_SERIALTERMINAL_HPP_

#include <algorithm>
#include <list>
#include <xcore/interface.h>
#include "Shell/Script.hpp"
#include "Shell/Terminal.hpp"

class SerialTerminal: public Terminal
{
public:
  SerialTerminal(Interface *interface) :
    m_interface{interface}
  {
    ifSetCallback(m_interface, dataCallbackHelper, this);
  }

  virtual ~SerialTerminal()
  {
    ifSetCallback(m_interface, nullptr, nullptr);
  }

  virtual void subscribe(Script *script) override
  {
    m_subscribers.push_back(script);
  }

  virtual void unsubscribe(const Script *script) override
  {
    auto iter = std::find(m_subscribers.begin(), m_subscribers.end(), script);

    if (iter != m_subscribers.end())
      m_subscribers.erase(iter);
  }

  virtual size_t read(char *buffer, size_t length) override
  {
    // TODO Locks
    return ifRead(m_interface, buffer, length);
  }

  virtual size_t write(const char *buffer, size_t length) override
  {
    // TODO Locks
    return ifWrite(m_interface, buffer, length);
  }

private:
  static constexpr size_t BUFFER_SIZE = 64;

  Interface *m_interface;
  std::list<Script *> m_subscribers;

  void dataCallback()
  {
    size_t available;

    if (ifGetParam(m_interface, IF_AVAILABLE, &available) == E_OK && available)
    {
      SerialInputEvent event;

      event.event = ScriptEvent::Event::SERIAL_INPUT;
      event.length = available;

      // TODO Locks
      for (auto script : m_subscribers)
        script->onEventReceived(&event);
    }
  }

  static void dataCallbackHelper(void *argument)
  {
    static_cast<SerialTerminal *>(argument)->dataCallback();
  }
};

#endif // VFS_SHELL_CORE_SHELL_SERIALTERMINAL_HPP_
