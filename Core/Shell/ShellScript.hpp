/*
 * Core/Shell/ShellScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SHELLSCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SHELLSCRIPT_HPP_

#include "Shell/Script.hpp"

class ShellScript: public Script
{
public:
  ShellScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    m_parent{parent},
    m_firstArgument{firstArgument},
    m_lastArgument{lastArgument}
  {
    m_parent->tty().subscribe(this);
  }

  virtual ~ShellScript()
  {
    m_parent->tty().unsubscribe(this);
  }

  virtual Result onEventReceived(const ScriptEvent *) override
  {
    return E_OK;
  }

  virtual Environment &env() override
  {
    return m_parent->env();
  }

  virtual FsHandle *fs() override
  {
    return m_parent->fs();
  }

  virtual TimeProvider &time() override
  {
    return m_parent->time();
  }

  virtual Terminal &tty() override
  {
    return m_parent->tty();
  }

protected:
  Script * const m_parent;
  const ArgumentIterator m_firstArgument;
  const ArgumentIterator m_lastArgument;
};

#endif // VFS_SHELL_CORE_SHELL_SHELLSCRIPT_HPP_
