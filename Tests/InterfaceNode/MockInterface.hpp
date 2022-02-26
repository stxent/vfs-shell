/*
 * Tests/InterfaceNode/MockInterface.hpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_TESTS_INTERFACENODE_MOCKINTERFACE_HPP_
#define VFS_SHELL_TESTS_INTERFACENODE_MOCKINTERFACE_HPP_

#include <xcore/containers/byte_queue.h>
#include <xcore/interface.h>
#include <cstdint>
#include <new>

extern const InterfaceClass * const MockInterface;

class MockInterface
{
public:
  MockInterface(const MockInterface &) = delete;
  MockInterface &operator=(const MockInterface &) = delete;

  static Result init(void *object, const void *)
  {
    new (object) MockInterface{};
    return E_OK;
  }

  static void deinit(void *object)
  {
    static_cast<MockInterface *>(object)->~MockInterface();
  }

  static Result getParam(void *object, int parameter, void *data)
  {
    return static_cast<MockInterface *>(object)->getParamImpl(parameter, data);
  }

  static Result setParam(void *object, int parameter, const void *data)
  {
    return static_cast<MockInterface *>(object)->setParamImpl(parameter, data);
  }

  static size_t read(void *object, void *buffer, size_t length)
  {
    return static_cast<MockInterface *>(object)->readImpl(buffer, length);
  }

  static size_t write(void *object, const void *buffer, size_t length)
  {
    return static_cast<MockInterface *>(object)->writeImpl(buffer, length);
  }

private:
  static constexpr size_t QUEUE_SIZE{1024};

  Interface m_base;
  ByteQueue m_queue;

  uint64_t m_address{0};
  uint64_t m_position{0};
  uint32_t m_rate{0};
  uint64_t m_size{0};
  size_t m_rxAvailable{0};
  size_t m_rxPending{0};
  size_t m_rxWatermark{0};
  size_t m_txAvailable{0};
  size_t m_txPending{0};
  size_t m_txWatermark{0};

  MockInterface();
  ~MockInterface();

  Result getParamImpl(int, void *);
  Result setParamImpl(int, const void *);
  size_t readImpl(void *, size_t);
  size_t writeImpl(const void *, size_t);
};

#endif // VFS_SHELL_TESTS_INTERFACENODE_MOCKINTERFACE_HPP_
