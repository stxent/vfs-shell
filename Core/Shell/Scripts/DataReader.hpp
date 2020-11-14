/*
 * Core/Shell/Scripts/DataReader.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_DATAREADER_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_DATAREADER_HPP_

#include "Shell/ShellScript.hpp"
#include "Wrappers/Semaphore.hpp"
#include <functional>

class DataReader: public ShellScript
{
public:
  DataReader(Script *, ArgumentIterator, ArgumentIterator);
  virtual Result onEventReceived(const ScriptEvent *) override;

protected:
  Os::Semaphore m_semaphore;

  bool isTerminateRequested();
  Result read(void *, FsNode *, size_t, size_t, size_t, std::function<Result (const void *, size_t)>);
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_DATAREADER_HPP_
