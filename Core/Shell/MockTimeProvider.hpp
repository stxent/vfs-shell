/*
 * Core/Shell/MockTimeProvider.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_MOCKTIMEPROVIDER_HPP_
#define VFS_SHELL_CORE_SHELL_MOCKTIMEPROVIDER_HPP_

#include "Shell/TimeProvider.hpp"

class MockTimeProvider: public TimeProvider
{
public:
  virtual time64_t microtime() override
  {
    return 0;
  }

  static MockTimeProvider &instance()
  {
    static MockTimeProvider object;
    return object;
  }
};

#endif // VFS_SHELL_CORE_SHELL_MOCKTIMEPROVIDER_HPP_
