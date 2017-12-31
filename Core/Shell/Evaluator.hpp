/*
 * Core/Shell/Evaluator.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_EVALUATOR_HPP_
#define VFS_SHELL_CORE_SHELL_EVALUATOR_HPP_

#include <memory>
#include "Shell/Script.hpp"
#include "Shell/ShellHelpers.hpp"
#include "Shell/TerminalProxy.hpp"

template<typename T>
class Evaluator: public Script
{
public:
  Evaluator(Script *parent, T firstArgument, T lastArgument) :
    m_parent{parent},
    m_firstArgument{firstArgument},
    m_lastArgument{lastArgument},
    m_inputPathArgument{extractInputPath(firstArgument, lastArgument)},
    m_outputPathArgument{extractOutputPath(firstArgument, lastArgument)},
    m_terminal{this, m_parent->tty(),
        m_inputPathArgument != lastArgument ? *(m_inputPathArgument + 1) : nullptr,
        m_outputPathArgument != lastArgument ? *(m_outputPathArgument + 1) : nullptr,
        m_outputPathArgument != lastArgument ? isAppendRequested(m_outputPathArgument) : false}
  {
    m_parent->tty().subscribe(this);
  }

  virtual ~Evaluator()
  {
    m_parent->tty().unsubscribe(this);
  }

  virtual Result onEventReceived(const ScriptEvent *event) override
  {
    return m_terminal.onEventReceived(event);
  }

  virtual Environment &env() override
  {
    return m_parent->env();
  }

  virtual FsHandle *fs() override
  {
    return m_parent->fs();
  }

  virtual Result run() override
  {
    FsNode * const node = ShellHelpers::openScript(m_parent->fs(), m_parent->env(), *m_firstArgument);
    T lastSignificantArgument = std::min(m_lastArgument, std::min(m_inputPathArgument, m_outputPathArgument));

    if (node != nullptr)
    {
      uint8_t header[ScriptHeaders::MAX_HEADER_SIZE];
      size_t count;
      Result res = E_INVALID;

      if (fsNodeRead(node, FS_NODE_DATA, 0, header, sizeof(header), &count) == E_OK && count == sizeof(header))
      {
        if (!memcmp(header, ScriptHeaders::OBJECT_HEADER, ScriptHeaders::OBJECT_HEADER_SIZE))
        {
          ScriptRunnerBase *runner;

          if (fsNodeRead(node, FS_NODE_DATA, static_cast<FsLength>(ScriptHeaders::OBJECT_HEADER_SIZE),
              &runner, sizeof(runner), &count) == E_OK && count == sizeof(runner))
          {
            res = runner->run(this, m_firstArgument + 1, lastSignificantArgument);
          }
        }
        else if (!memcmp(header, ScriptHeaders::SCRIPT_HEADER, ScriptHeaders::SCRIPT_HEADER_SIZE))
        {

        }
      }

      fsNodeFree(node);
      return res;
    }
    else
      return E_ENTRY;
  }

  virtual TimeProvider &time() override
  {
    return m_parent->time();
  }

  virtual Terminal &tty() override
  {
    return m_terminal;
  }

private:
  Script * const m_parent;
//  T firstArgument;
//  T lastArgument;
//  T inputPathArgument;
//  T outputPathArgument;
  const T m_firstArgument;
  const T m_lastArgument;
  const T m_inputPathArgument;
  const T m_outputPathArgument;
  TerminalProxy m_terminal;

  static T extractInputPath(T firstArgument, T lastArgument)
  {
//    auto argument = std::find_if(first, last, [](T entry){ return !strcmp(*entry, "<"); });
    auto argument = firstArgument;
    while (argument != lastArgument)
    {
      if (!strcmp(*argument, "<"))
        break;
      ++argument;
    }
    return (argument != lastArgument && argument + 1 != lastArgument) ? argument : lastArgument;
  }

  static T extractOutputPath(T firstArgument, T lastArgument)
  {
//    auto argument = std::find_if(first, last, [](T entry){ return !strcmp(*entry, ">") || !strcmp(*entry, ">>"); });
    auto argument = firstArgument;
    while (argument != lastArgument)
    {
      if (!strcmp(*argument, ">") || !strcmp(*argument, ">>"))
        break;
      ++argument;
    }
    return (argument != lastArgument && argument + 1 != lastArgument) ? argument : lastArgument;
  }

  static bool isAppendRequested(T argument)
  {
    return !strcmp(*argument, ">>");
  }
};

#endif // VFS_SHELL_CORE_SHELL_EVALUATOR_HPP_
