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

  virtual time64_t getAlarm() override;
  virtual time64_t getTime() override;
  virtual Result setAlarm(time64_t) override;
  virtual Result setTime(time64_t) override;

  static UnixTimeProvider &instance()
  {
    static UnixTimeProvider object;
    return object;
  }

private:
  UnixTimeProvider() = default;
};

#endif // VFS_SHELL_PLATFORM_LINUX_UNIXTIMEPROVIDER_HPP_
