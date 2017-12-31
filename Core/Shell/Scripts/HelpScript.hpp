/*
 * Core/Shell/Scripts/HelpScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_HELPSCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_HELPSCRIPT_HPP_

#include <iterator>
#include "Shell/ShellHelpers.hpp"
#include "Shell/ShellScript.hpp"

class HelpScript: public ShellScript
{
public:
  HelpScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    ShellScript{parent, firstArgument, lastArgument}
  {
  }

  virtual Result run() override
  {
    const char * const arguments[] = {"ls", env()["PATH"]};
    return Evaluator<ArgumentIterator>{this, std::cbegin(arguments), std::cend(arguments)}.run();
  }

  static const char *name()
  {
    return "man";
  }
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_HELPSCRIPT_HPP_
