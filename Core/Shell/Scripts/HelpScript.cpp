/*
 * HelpScript.cpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <iterator>
#include "Shell/Evaluator.hpp"
#include "Shell/Scripts/HelpScript.hpp"
#include "Shell/ShellHelpers.hpp"

HelpScript::HelpScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
  ShellScript{parent, firstArgument, lastArgument}
{
}

Result HelpScript::run()
{
  const char * const arguments[] = {"ls", env()["PATH"]};
  return Evaluator<ArgumentIterator>{this, std::cbegin(arguments), std::cend(arguments)}.run();
}
