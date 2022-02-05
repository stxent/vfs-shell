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
  MockTimeProvider() :
    m_alarm{0},
    m_timestamp{0}
  {
  }

  virtual time64_t getAlarm() override
  {
    return m_alarm;
  }

  virtual time64_t getTime() override
  {
    return m_timestamp;
  }

  virtual Result setAlarm(time64_t timestamp) override
  {
    m_alarm = timestamp;
    return E_OK;
  }

  virtual Result setTime(time64_t timestamp) override
  {
    m_timestamp = timestamp;
    return E_OK;
  }

  static MockTimeProvider &instance()
  {
    static MockTimeProvider object;
    return object;
  }

private:
  time64_t m_alarm;
  time64_t m_timestamp;
};

#endif // VFS_SHELL_CORE_SHELL_MOCKTIMEPROVIDER_HPP_
