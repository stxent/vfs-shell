/*
 * Tests/InterfaceNode/MockSerial.hpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_TESTS_INTERFACENODE_MOCKSERIAL_HPP_
#define VFS_SHELL_TESTS_INTERFACENODE_MOCKSERIAL_HPP_

#include <halm/generic/serial.h>
#include <xcore/containers/byte_queue.h>
#include <new>

extern const InterfaceClass * const MockSerial;

class MockSerial
{
public:
  MockSerial(const MockSerial &) = delete;
  MockSerial &operator=(const MockSerial &) = delete;

  static Result init(void *object, const void *)
  {
    new (object) MockSerial{};
    return E_OK;
  }

  static void deinit(void *object)
  {
    static_cast<MockSerial *>(object)->~MockSerial();
  }

  static Result getParam(void *object, int parameter, void *data)
  {
    return static_cast<MockSerial *>(object)->getParamImpl(parameter, data);
  }

  static Result setParam(void *object, int parameter, const void *data)
  {
    return static_cast<MockSerial *>(object)->setParamImpl(parameter, data);
  }

  static size_t read(void *object, void *buffer, size_t length)
  {
    return static_cast<MockSerial *>(object)->readImpl(buffer, length);
  }

  static size_t write(void *object, const void *buffer, size_t length)
  {
    return static_cast<MockSerial *>(object)->writeImpl(buffer, length);
  }

private:
  static constexpr size_t QUEUE_SIZE{1024};

  Interface m_base;
  ByteQueue m_queue;

  SerialParity m_parity{SERIAL_PARITY_NONE};
  bool m_cts{false};
  bool m_dsr{false};
  bool m_dtr{false};
  bool m_rts{false};

  MockSerial();
  ~MockSerial();

  Result getParamImpl(int, void *);
  Result setParamImpl(int, const void *);
  size_t readImpl(void *, size_t);
  size_t writeImpl(const void *, size_t);
};

#endif // VFS_SHELL_TESTS_INTERFACENODE_MOCKSERIAL_HPP_
