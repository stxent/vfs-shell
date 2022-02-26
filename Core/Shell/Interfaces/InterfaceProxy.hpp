/*
 * Core/Shell/Interfaces/InterfaceProxy.hpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_INTERFACES_INTERFACEPROXY_HPP_
#define VFS_SHELL_CORE_SHELL_INTERFACES_INTERFACEPROXY_HPP_

#include <xcore/interface.h>
#include <new>

extern const InterfaceClass * const InterfaceProxy;

class InterfaceProxy
{
public:
  struct Config
  {
    /** Mandatory: underlying interface. */
    Interface *pipe;
  };

  InterfaceProxy(const InterfaceProxy &) = delete;
  InterfaceProxy &operator=(const InterfaceProxy &) = delete;

  static Result init(void *object, const void *configBase)
  {
    const Config * const config = static_cast<const Config *>(configBase);
    new (object) InterfaceProxy{config->pipe};
    return E_OK;
  }

  static void deinit(void *object)
  {
    static_cast<InterfaceProxy *>(object)->~InterfaceProxy();
  }

  static Result getParam(void *object, int parameter, void *data)
  {
    return static_cast<InterfaceProxy *>(object)->getParamImpl(parameter, data);
  }

  static Result setParam(void *object, int parameter, const void *data)
  {
    return static_cast<InterfaceProxy *>(object)->setParamImpl(parameter, data);
  }

  static size_t read(void *object, void *buffer, size_t length)
  {
    return static_cast<InterfaceProxy *>(object)->readImpl(buffer, length);
  }

  static size_t write(void *object, const void *buffer, size_t length)
  {
    return static_cast<InterfaceProxy *>(object)->writeImpl(buffer, length);
  }

private:
  Interface m_base;
  Interface * const m_interface;

  InterfaceProxy(Interface *interface) :
    m_interface{interface}
  {
    // m_base should be left untouched
  }

  Result getParamImpl(int parameter, void *data)
  {
    return ifGetParam(m_interface, parameter, data);
  }

  Result setParamImpl(int parameter, const void *data)
  {
    return ifSetParam(m_interface, parameter, data);
  }

  size_t readImpl(void *buffer, size_t length)
  {
    return ifRead(m_interface, buffer, length);
  }

  size_t writeImpl(const void *buffer, size_t length)
  {
    return ifWrite(m_interface, buffer, length);
  }
};

#endif // VFS_SHELL_CORE_SHELL_INTERFACES_INTERFACEPROXY_HPP_
