/*
 * Tests/Shared/VirtualMem.hpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_TESTS_SHARED_VIRTUALMEM_HPP_
#define VFS_SHELL_TESTS_SHARED_VIRTUALMEM_HPP_

#include <xcore/interface.h>
#include <cstdint>
#include <memory>
#include <new>

extern const InterfaceClass * const VirtualMem;

class VirtualMem
{
public:
  struct Config
  {
    /** Mandatory: partition size. */
    size_t size;
  };

  VirtualMem(const VirtualMem &) = delete;
  VirtualMem &operator=(const VirtualMem &) = delete;

  static Result init(void *object, const void *configBase)
  {
    const auto config = static_cast<const Config *>(configBase);

    new (object) VirtualMem{config->size};
    return E_OK;
  }

  static void deinit(void *object)
  {
    static_cast<VirtualMem *>(object)->~VirtualMem();
  }

  static Result getParam(void *object, int parameter, void *data)
  {
    return static_cast<VirtualMem *>(object)->getParamImpl(parameter, data);
  }

  static Result setParam(void *object, int parameter, const void *data)
  {
    return static_cast<VirtualMem *>(object)->setParamImpl(parameter, data);
  }

  static size_t read(void *object, void *buffer, size_t length)
  {
    return static_cast<VirtualMem *>(object)->readImpl(buffer, length);
  }

  static size_t write(void *object, const void *buffer, size_t length)
  {
    return static_cast<VirtualMem *>(object)->writeImpl(buffer, length);
  }

  uint8_t *arena()
  {
    return m_data.get();
  }

private:
  Interface m_base;

  std::unique_ptr<uint8_t []> m_data{nullptr};
  size_t m_position{0};
  size_t m_size;

  VirtualMem(size_t);
  ~VirtualMem();

  Result getParamImpl(int, void *);
  Result setParamImpl(int, const void *);
  size_t readImpl(void *, size_t);
  size_t writeImpl(const void *, size_t);
};

#endif // VFS_SHELL_TESTS_SHARED_VIRTUALMEM_HPP_
