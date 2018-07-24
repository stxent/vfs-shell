/*
 * EchoScript.cpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Shell/Scripts/EchoScript.hpp"

EchoScript::EchoScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
  ShellScript{parent, firstArgument, lastArgument}
{
}

Result EchoScript::run()
{
  for (auto iter = m_firstArgument; iter != m_lastArgument; ++iter)
  {
    tty() << *iter;

    if (iter != m_lastArgument - 1)
      tty() << " ";
  }
  tty() << Terminal::EOL;

  return E_OK;
}
