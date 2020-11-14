/*
 * ChangeDirectoryScript.cpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Shell/ArgParser.hpp"
#include "Shell/Scripts/ChangeDirectoryScript.hpp"
#include "Shell/ShellHelpers.hpp"
#include "Shell/Settings.hpp"
#include <xcore/fs/utils.h>
#include <iterator>

ChangeDirectoryScript::ChangeDirectoryScript(Script *parent, ArgumentIterator firstArgument,
    ArgumentIterator lastArgument) :
  ShellScript{parent, firstArgument, lastArgument}
{
}

Result ChangeDirectoryScript::run()
{
  static const ArgParser::Descriptor descriptors[] = {
      {"--help", nullptr, "show this help message and exit", 0, Arguments::helpSetter},
      {nullptr, "ENTRY", "change current directory to ENTRY", 1, Arguments::pathSetter}
  };

  bool argumentsParsed;
  const Arguments arguments = ArgParser::parse<Arguments>(m_firstArgument, m_lastArgument,
      std::cbegin(descriptors), std::cend(descriptors), &argumentsParsed);

  if (arguments.help)
  {
    ArgParser::help(tty(), name(), std::cbegin(descriptors), std::cend(descriptors));
    return E_OK;
  }
  else if (argumentsParsed)
  {
    return changeDirectory(arguments.path);
  }
  else
  {
    return E_VALUE;
  }
}

Result ChangeDirectoryScript::changeDirectory(const char *relativePath)
{
  char path[Settings::PWD_LENGTH];

  fsJoinPaths(path, env()["PWD"], relativePath);
  FsNode * const root = fsOpenNode(fs(), path);
  if (root == nullptr)
  {
    tty() << name() << ": " << relativePath << ": no such node" << Terminal::EOL;
    return E_ENTRY;
  }

  FsAccess access;
  const Result res = fsNodeRead(root, FS_NODE_ACCESS, 0, &access, sizeof(access), nullptr);
  fsNodeFree(root);

  if (res != E_OK)
  {
    tty() << name() << ": " << relativePath << ": error reading access attribute" << Terminal::EOL;
    return res;
  }

  // TODO Hierarchical check
  if (!(access & FS_ACCESS_READ))
  {
    tty() << name() << ": " << relativePath << ": permission denied" << Terminal::EOL;
    return E_ACCESS;
  }

  env()["PWD"] = path;
  return E_OK;
}
