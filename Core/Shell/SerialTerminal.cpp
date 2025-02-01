/*
 * SerialTerminal.cpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Shell/Script.hpp"
#include "Shell/SerialTerminal.hpp"
#include <algorithm>

SerialTerminal::SerialTerminal(Interface *interface, bool coloration) :
  Terminal{coloration},
  m_interface{interface}
{
  ifSetCallback(m_interface, dataCallbackHelper, this);
}

SerialTerminal::~SerialTerminal()
{
  ifSetCallback(m_interface, nullptr, nullptr);
}

void SerialTerminal::subscribe(Script *script)
{
  Os::MutexLocker locker{m_lock};
  m_subscribers.push_back(script);
}

void SerialTerminal::unsubscribe(const Script *script)
{
  Os::MutexLocker locker{m_lock};
  auto iter = std::find(m_subscribers.begin(), m_subscribers.end(), script);

  if (iter != m_subscribers.end())
    m_subscribers.erase(iter);
}

size_t SerialTerminal::read(char *buffer, size_t length)
{
  return ifRead(m_interface, buffer, length);
}

size_t SerialTerminal::write(const char *buffer, size_t length)
{
  const char *position = buffer;
  size_t left = length;

  while (left)
  {
    const size_t bytesWritten = ifWrite(m_interface, position, left);
    left -= bytesWritten;
    position += bytesWritten;
  }

  return length;
}

void SerialTerminal::dataCallback()
{
  size_t available;

  if (ifGetParam(m_interface, IF_RX_AVAILABLE, &available) == E_OK && available)
  {
    Os::MutexLocker locker{m_lock};
    SerialInputEvent event;

    event.event = ScriptEvent::Event::SERIAL_INPUT;
    event.length = available;

    for (auto script : m_subscribers)
      script->onEventReceived(&event);
  }
}
