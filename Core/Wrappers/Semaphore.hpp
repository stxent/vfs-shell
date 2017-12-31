/*
 * Wrappers/Semaphore.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_WRAPPERS_SEMAPHORE_HPP_
#define VFS_SHELL_WRAPPERS_SEMAPHORE_HPP_

namespace Osw
{
#include <osw/semaphore.h>
}

class Semaphore
{
public:
  Semaphore(int value)
  {
    if (Osw::semInit(&m_sem, value) != E_OK)
      exit(EXIT_FAILURE);
  }

  ~Semaphore()
  {
    Osw::semDeinit(&m_sem);
  }

  void post()
  {
    Osw::semPost(&m_sem);
  }

  bool tryWait(unsigned int interval)
  {
    return Osw::semTryWait(&m_sem, interval);
  }

  int value()
  {
    return Osw::semValue(&m_sem);
  }

  void wait()
  {
    Osw::semWait(&m_sem);
  }

private:
  Osw::Semaphore m_sem;
};

#endif // VFS_SHELL_WRAPPERS_SEMAPHORE_HPP_
