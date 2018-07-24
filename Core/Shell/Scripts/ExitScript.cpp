/*
 * ExitScript.cpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Shell/Scripts/ExitScript.hpp"

ExitScript::ExitScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
  ShellScript{parent, firstArgument, lastArgument}
{
}

Result ExitScript::run()
{
  // TODO
  return E_OK;
}
