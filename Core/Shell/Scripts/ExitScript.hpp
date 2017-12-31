/*
 * Core/Shell/Scripts/ExitScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_EXITSCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_EXITSCRIPT_HPP_

#include "Shell/ShellScript.hpp"

class ExitScript: public ShellScript
{
public:
  ExitScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    ShellScript{parent, firstArgument, lastArgument}
  {
  }

  virtual Result run() override
  {
//    SignalEvent event;
//
//    event.event = ScriptEvent::Event::SIGNAL;
//    event.signal = SIGINT;
//
//    shell->onEventReceived(&event); //TODO
    return E_OK;
  }

  static const char *name()
  {
    return "exit";
  }
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_EXITSCRIPT_HPP_
