/*
 * Core/Shell/TimeProvider.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_TIMEPROVIDER_HPP_
#define VFS_SHELL_CORE_SHELL_TIMEPROVIDER_HPP_

#include <xcore/realtime.h>

class TimeProvider
{
public:
  virtual ~TimeProvider() = default;
  virtual time64_t getTime() = 0;
  virtual Result setAlarm(time64_t) = 0;
  virtual Result setTime(time64_t) = 0;
};

#endif // VFS_SHELL_CORE_SHELL_TIMEPROVIDER_HPP_
