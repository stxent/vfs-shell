/*
 * Wrappers/Mutex.hpp
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_WRAPPERS_MUTEX_HPP_
#define VFS_SHELL_WRAPPERS_MUTEX_HPP_

#include <xcore/os/mutex.h>
#include <cstdlib>

namespace Os
{

class Mutex
{
public:
  Mutex()
  {
    if (mutexInit(&m_mut) != E_OK)
      exit(EXIT_FAILURE);
  }

  ~Mutex()
  {
    mutexDeinit(&m_mut);
  }

  void lock()
  {
    mutexLock(&m_mut);
  }

  bool tryLock(unsigned int interval = 0)
  {
    return mutexTryLock(&m_mut, interval);
  }

  void unlock()
  {
    mutexUnlock(&m_mut);
  }

private:
  ::Mutex m_mut;
};

class MutexLocker {
public:
	MutexLocker(Mutex &parent) :
		m_parent{parent}
	{
		m_parent.lock();
	}

	~MutexLocker()
	{
		m_parent.unlock();
	}

private:
	Mutex &m_parent;
};

} // namespace Os

#endif // VFS_SHELL_WRAPPERS_MUTEX_HPP_
