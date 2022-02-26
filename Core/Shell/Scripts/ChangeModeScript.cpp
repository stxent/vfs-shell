/*
 * ChangeModeScript.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Shell/Scripts/ChangeModeScript.hpp"
#include "Shell/Settings.hpp"
#include <xcore/fs/utils.h>

const std::array<ArgParser::Descriptor, 4> ChangeModeScript::descriptors{
    {
        {"--help", nullptr, "show this help message and exit", 0, Arguments::helpSetter},
        {"-R", nullptr, "change mode recursively", 0, Arguments::recursiveSetter},
        {nullptr, "MODE", "mode in a symbolic format", 1, Arguments::modeSetter},
        {nullptr, "FILE", "change mode bits for FILE", 0, Arguments::incrementNodeCount}
    }
};

ChangeModeScript::ChangeModeScript(Script *parent, ArgumentIterator firstArgument,
    ArgumentIterator lastArgument) :
  ShellScript{parent, firstArgument, lastArgument},
  m_arguments{ArgParser::parse<Arguments>(m_firstArgument, m_lastArgument,
      descriptors.cbegin(), descriptors.cend())},
  m_result{E_OK}
{
}

Result ChangeModeScript::run()
{
  if (m_arguments.help)
  {
    ArgParser::help(tty(), name(), std::cbegin(descriptors), std::cend(descriptors));
    return E_OK;
  }
  else if (m_arguments.count)
  {
    ArgParser::invoke(m_firstArgument, m_lastArgument, descriptors.cbegin(), descriptors.cend(),
        [this](const char *key){ changeEntryMode(key); });

    return m_result;
  }
  else
    return E_VALUE;
}

void ChangeModeScript::changeEntryMode(const char *positionalArgument)
{
  // TODO Recursive mode set

  if (m_result != E_OK)
    return;

  char path[Settings::PWD_LENGTH];
  fsJoinPaths(path, env()["PWD"], positionalArgument);

  FsNode * const node = fsOpenNode(fs(), path);
  if (node == nullptr)
  {
    tty() << name() << ": " << positionalArgument << ": node not found" << Terminal::EOL;
    m_result = E_ENTRY;
    return;
  }

  FsAccess access{0};
  Result res;

  res = fsNodeRead(node, FS_NODE_ACCESS, 0, &access, sizeof(access), nullptr);
  if (res == E_OK)
  {
    access = (access & ~m_arguments.modeClear) | m_arguments.modeSet;
    res = fsNodeWrite(node, FS_NODE_ACCESS, 0, &access, sizeof(access), nullptr);
  }
  fsNodeFree(node);

  if (res != E_OK)
  {
    tty() << name() << ": " << positionalArgument << ": changing permissions failed" << Terminal::EOL;
    m_result = res;
  }
}

void ChangeModeScript::Arguments::helpSetter(void *object, const char *)
{
  static_cast<Arguments *>(object)->help = true;
}

void ChangeModeScript::Arguments::incrementNodeCount(void *object, const char *)
{
  ++static_cast<Arguments *>(object)->count;
}

void ChangeModeScript::Arguments::modeSetter(void *object, const char *argument)
{
  enum class State
  {
    NONE,
    SET,
    CLEAR
  };

  auto args = static_cast<Arguments *>(object);
  State state = State::NONE;

  args->modeClear = 0;
  args->modeSet = 0;

  for (size_t i = 0; i < strlen(argument); ++i)
  {
    if (argument[i] == '-')
    {
      state = State::CLEAR;
    }
    else if (argument[i] == '+')
    {
      state = State::SET;
    }
    else if (state != State::NONE)
    {
      if (argument[i] == 'r')
      {
        if (state == State::SET)
          args->modeSet |= FS_ACCESS_READ;
        else
          args->modeClear |= FS_ACCESS_READ;
      }
      else if (argument[i] == 'w')
      {
        if (state == State::SET)
          args->modeSet |= FS_ACCESS_WRITE;
        else
          args->modeClear |= FS_ACCESS_WRITE;
      }
    }
  }
}

void ChangeModeScript::Arguments::recursiveSetter(void *object, const char *)
{
  static_cast<Arguments *>(object)->recursive = true;
}
