/*
 * Core/Shell/Initializer.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_INITIALIZER_HPP_
#define VFS_SHELL_CORE_SHELL_INITIALIZER_HPP_

#include <cassert>
#include <atomic>
#include <vector>
#include "Shell/Evaluator.hpp"
#include "Shell/Settings.hpp"

class Initializer: public Script
{
public:
  Initializer(FsHandle *handle, Terminal &terminal, TimeProvider &clock, bool echoing) :
    m_handle{handle},
    m_clock{clock},
    m_terminal{terminal}
  {
    m_environment.make<StaticEnvironmentVariable<Settings::PWD_LENGTH>>("PATH") = "/bin";
    m_environment.make<StaticEnvironmentVariable<Settings::PWD_LENGTH>>("PWD") = "/";

    m_environment["DEBUG"] = "0";
    m_environment["ECHO"] = echoing ? "1" : "0";
    m_environment["SHELL"] = "sh";
  }

  virtual ~Initializer()
  {
    FsNode * const binEntryNode = ShellHelpers::openNode(fs(), env()["PATH"]);
    assert(binEntryNode != nullptr);

    // XXX Code assumes that PATH contains default directory
    for (auto iter = m_scripts.begin(); iter != m_scripts.end(); ++iter)
    {
      FsNode * const scriptEntryNode = ShellHelpers::openScript(fs(), env(), (*iter)->name());

      if (scriptEntryNode != nullptr)
      {
        fsNodeRemove(binEntryNode, scriptEntryNode);
        fsNodeFree(scriptEntryNode);
      }
    }

    fsNodeFree(binEntryNode);
  }

  template<typename T, typename... ARGs>
  void attach(ARGs... args)
  {
    FsNode * const binEntryNode = ShellHelpers::openNode(fs(), env()["PATH"]);
    assert(binEntryNode != nullptr);

    ScriptRunnerBase * const runner = new ScriptRunner<T, ARGs...>{args...};
    uint8_t wrapper[ScriptHeaders::OBJECT_HEADER_SIZE + sizeof(runner)];
    memcpy(wrapper, ScriptHeaders::OBJECT_HEADER, sizeof(ScriptHeaders::OBJECT_HEADER));
    memcpy(wrapper + ScriptHeaders::OBJECT_HEADER_SIZE, &runner, sizeof(runner));

    const auto nodeTime = time().getTime();
    const FsFieldDescriptor runnerEntryFields[] = {
        {T::name(), strlen(T::name()) + 1, FS_NODE_NAME},
        {&nodeTime, sizeof(nodeTime), FS_NODE_TIME},
        {wrapper, sizeof(wrapper), FS_NODE_DATA}
    };
    fsNodeCreate(binEntryNode, runnerEntryFields, ARRAY_SIZE(runnerEntryFields));
    fsNodeFree(binEntryNode);

    m_scripts.emplace_back(runner);
  }

  virtual Environment &env() override
  {
    return m_environment;
  }

  virtual FsHandle *fs() override
  {
    return m_handle;
  }

  virtual TimeProvider &time() override
  {
    return m_clock;
  }

  virtual Terminal &tty() override
  {
    return m_terminal;
  }

  virtual Result onEventReceived(const ScriptEvent *) override
  {
    return E_INVALID;
  }

  virtual Result run() override
  {
    const char * const arguments[] = {m_environment["SHELL"]};
    return Evaluator<ArgumentIterator>{this, std::cbegin(arguments), std::cend(arguments)}.run();
  }

private:
  FsHandle *m_handle;
  TimeProvider &m_clock;
  Terminal &m_terminal;
  Environment m_environment;

  std::vector<std::unique_ptr<ScriptRunnerBase>> m_scripts;
};

#endif // VFS_SHELL_CORE_SHELL_INITIALIZER_HPP_
