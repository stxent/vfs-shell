/*
 * Core/Shell/Scripts/TimeScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_TIMESCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_TIMESCRIPT_HPP_

#include "Shell/Evaluator.hpp"

class TimeScript: public ShellScript
{
public:
  TimeScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    ShellScript{parent, firstArgument, lastArgument}
  {
  }

  virtual Result run() override
  {
    const auto start = time().get();
    Evaluator<ArgumentIterator> evaluator{this, m_firstArgument, m_lastArgument};
    const Result res = evaluator.run();
    const auto delta = time().get() - start;

    // TODO Save and restore fill and width
    tty() << "elapsed " << delta / 1000000 << "." << Terminal::Fill{'0'} << Terminal::Width{6} << delta % 1000000
        << Terminal::Width{1} << Terminal::Fill{' '} << " s" << Terminal::EOL;

    return res;
  }

  static const char *name()
  {
    return "time";
  }
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_TIMESCRIPT_HPP_
