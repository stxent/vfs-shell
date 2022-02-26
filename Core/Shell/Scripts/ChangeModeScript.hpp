/*
 * Core/Shell/Scripts/ChangeModeScript.hpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_CHANGEMODESCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_CHANGEMODESCRIPT_HPP_

#include "Shell/ArgParser.hpp"
#include "Shell/ShellScript.hpp"
#include <array>

class ChangeModeScript: public ShellScript
{
public:
  ChangeModeScript(Script *, ArgumentIterator, ArgumentIterator);
  virtual Result run() override;

  static const char *name()
  {
    return "chmod";
  }

private:
  struct Arguments
  {
    size_t count{0};
    FsAccess modeClear{0};
    FsAccess modeSet{0};
    bool help{false};
    bool recursive{false};

    static void helpSetter(void *, const char *);
    static void incrementNodeCount(void *, const char *);
    static void modeSetter(void *, const char *);
    static void recursiveSetter(void *, const char *);
  };

  const Arguments m_arguments;
  Result m_result;

  void changeEntryMode(const char *);

  static const std::array<ArgParser::Descriptor, 4> descriptors;
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_CHANGEMODESCRIPT_HPP_
