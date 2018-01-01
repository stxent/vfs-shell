/*
 * Platform/Linux/UnixTimeProvider.hpp
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_PLATFORM_LINUX_UNIXTIMEPROVIDER_HPP_
#define VFS_SHELL_PLATFORM_LINUX_UNIXTIMEPROVIDER_HPP_

#include "Shell/TimeProvider.hpp"

class UnixTimeProvider: public TimeProvider
{
public:
  UnixTimeProvider(const UnixTimeProvider &) = delete;
  UnixTimeProvider &operator=(const UnixTimeProvider &) = delete;

  virtual ~UnixTimeProvider();

  virtual time64_t get() override;

  virtual Result set(time64_t) override
  {
    return E_INVALID;
  }

  static UnixTimeProvider &instance()
  {
    static UnixTimeProvider object;
    return object;
  }

private:
  UnixTimeProvider();

  RtClock *clock;
};

#endif // VFS_SHELL_PLATFORM_LINUX_UNIXTIMEPROVIDER_HPP_
