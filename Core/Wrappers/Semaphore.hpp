/*
 * Wrappers/Semaphore.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_WRAPPERS_SEMAPHORE_HPP_
#define VFS_SHELL_WRAPPERS_SEMAPHORE_HPP_

#include <osw/semaphore.h>
#include <cstdlib>

namespace Os
{

class Semaphore
{
public:
  Semaphore(int value)
  {
    if (semInit(&m_sem, value) != E_OK)
      exit(EXIT_FAILURE);
  }

  ~Semaphore()
  {
    semDeinit(&m_sem);
  }

  void post()
  {
    semPost(&m_sem);
  }

  bool tryWait(unsigned int interval = 0)
  {
    return semTryWait(&m_sem, interval);
  }

  int value()
  {
    return semValue(&m_sem);
  }

  void wait()
  {
    semWait(&m_sem);
  }

private:
  ::Semaphore m_sem;
};

} // namespace Os

#endif // VFS_SHELL_WRAPPERS_SEMAPHORE_HPP_
