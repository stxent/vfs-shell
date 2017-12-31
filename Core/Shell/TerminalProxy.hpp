/*
 * Core/Shell/TerminalProxy.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_TERMINALPROXY_HPP_
#define VFS_SHELL_CORE_SHELL_TERMINALPROXY_HPP_

#include <memory>
#include "Shell/ShellScript.hpp"

class TerminalProxy: public Terminal
{
public:
  TerminalProxy(Script *parent, Terminal &terminal, const char *inputPath = nullptr,
      const char *outputPath = nullptr, bool append = false) :
    m_terminal{terminal},
    m_parent{parent},
    m_subscriber{nullptr},
    m_input{nullptr, 0},
    m_output{nullptr, 0}
  {
    // TODO Better error handling
    if (inputPath != nullptr)
    {
      m_input.node = {
          ShellHelpers::openSource(m_parent->fs(), m_parent->env(), inputPath),
          freeNode
      };
    }

    if (outputPath != nullptr)
    {
      m_output.node = {
          ShellHelpers::openSink(m_parent->fs(), m_parent->env(), m_parent->time(), outputPath, true, nullptr),
          freeNode
      };

      if (m_output.node != nullptr && append)
        fsNodeLength(m_output.node.get(), FS_NODE_DATA, &m_output.position);
    }
  }

  virtual void subscribe(Script *script) override
  {
    m_subscriber = script;
  }

  virtual void unsubscribe(const Script *script) override
  {
    if (m_subscriber == script)
      m_subscriber = nullptr;
  }

  virtual size_t read(char *buffer, size_t length) override
  {
    if (m_input.node != nullptr)
    {
      size_t count;

      if (fsNodeRead(m_input.node.get(), FsFieldType::FS_NODE_DATA, m_input.position, buffer, length,
          &count) == E_OK)
      {
        m_input.position += static_cast<FsLength>(count);
        return count;
      }
      else
        return 0;
    }
    else
    {
      return m_terminal.read(buffer, length);
    }
  }

  virtual size_t write(const char *buffer, size_t length) override
  {
    if (m_output.node != nullptr)
    {
      size_t count;

      if (fsNodeWrite(m_output.node.get(), FsFieldType::FS_NODE_DATA, m_output.position, buffer, length,
          &count) == E_OK)
      {
        m_output.position += static_cast<FsLength>(count);
        return count;
      }
      else
        return 0;
    }
    else
    {
      return m_terminal.write(buffer, length);
    }
  }

  Result onEventReceived(const ScriptEvent *event)
  {
    return m_subscriber != nullptr ? m_subscriber->onEventReceived(event) : E_ERROR;
  }

private:
  static void freeNode(FsNode *node)
  {
    fsNodeFree(node);
  }

  Terminal &m_terminal;
  Script *m_parent;
  Script *m_subscriber;

  struct
  {
    std::unique_ptr<FsNode, std::function<void (FsNode *)>> node;
    FsLength position;
  } m_input;

  struct
  {
    std::unique_ptr<FsNode, std::function<void (FsNode *)>> node;
    FsLength position;
  } m_output;
};

#endif // VFS_SHELL_CORE_SHELL_TERMINALPROXY_HPP_
