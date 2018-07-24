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
  TerminalProxy(Script *, Terminal &, const char * = nullptr, const char * = nullptr, bool = false);
  Result onEventReceived(const ScriptEvent *);

  virtual void subscribe(Script *) override;
  virtual void unsubscribe(const Script *) override;
  virtual size_t read(char *, size_t) override;
  virtual size_t write(const char *, size_t) override;

private:
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

  static void freeNode(FsNode *);
};

#endif // VFS_SHELL_CORE_SHELL_TERMINALPROXY_HPP_
