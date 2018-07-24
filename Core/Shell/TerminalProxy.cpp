/*
 * TerminalProxy.cpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Shell/ShellHelpers.hpp"
#include "Shell/TerminalProxy.hpp"

TerminalProxy::TerminalProxy(Script *parent, Terminal &terminal, const char *inputPath,
    const char *outputPath, bool append) :
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

Result TerminalProxy::onEventReceived(const ScriptEvent *event)
{
  return m_subscriber != nullptr ? m_subscriber->onEventReceived(event) : E_ERROR;
}

void TerminalProxy::subscribe(Script *script)
{
  m_subscriber = script;
}

void TerminalProxy::unsubscribe(const Script *script)
{
  if (m_subscriber == script)
    m_subscriber = nullptr;
}

size_t TerminalProxy::read(char *buffer, size_t length)
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

size_t TerminalProxy::write(const char *buffer, size_t length)
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

void TerminalProxy::freeNode(FsNode *node)
{
  fsNodeFree(node);
}
