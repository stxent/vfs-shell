/*
 * TimeScript.cpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Shell/Evaluator.hpp"
#include "Shell/Scripts/TimeScript.hpp"

TimeScript::TimeScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
  ShellScript{parent, firstArgument, lastArgument}
{
}

Result TimeScript::run()
{
  const auto start = time().getTime();
  Evaluator<ArgumentIterator> evaluator{this, m_firstArgument, m_lastArgument};
  const Result res = evaluator.run();
  const auto delta = time().getTime() - start;

  const auto fill = tty().fill();
  const auto width = tty().width();

  tty() << "elapsed ";
  tty() << delta / 1000000 << "." << Terminal::Fill{'0'} << Terminal::Width{6} << delta % 1000000;
  tty() << " s" << Terminal::EOL;
  tty() << width << fill;

  return res;
}
