/*
 * Platform/LPC17xx_DevKit/InterfaceWrapper.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_INTERFACEWRAPPER_HPP_
#define VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_INTERFACEWRAPPER_HPP_

#include <halm/pin.h>
#include <xcore/interface.h>
#include <cassert>
#include <new>

extern const struct InterfaceClass * const InterfaceWrapper;

class InterfaceWrapper
{
public:
  struct Config
  {
    /** Mandatory: underlying interface. */
    Interface *pipe;
    /** Mandatory: reception indication. */
    PinNumber rx;
    /** Mandatory: transmission indication. */
    PinNumber tx;
  };

  InterfaceWrapper(const InterfaceWrapper &) = delete;
  InterfaceWrapper &operator=(const InterfaceWrapper &) = delete;

  static Result init(void *object, const void *configBase)
  {
    const Config * const config = static_cast<const Config *>(configBase);
    new (object) InterfaceWrapper{config->pipe, config->rx, config->tx};
    return E_OK;
  }

  static void deinit(void *object)
  {
    static_cast<InterfaceWrapper *>(object)->~InterfaceWrapper();
  }

  static void setCallback(void *object, void (*callback)(void *), void *argument)
  {
    static_cast<InterfaceWrapper *>(object)->setCallbackImpl(callback, argument);
  }

  static Result getParam(void *object, int parameter, void *data)
  {
    return static_cast<InterfaceWrapper *>(object)->getParamImpl(parameter, data);
  }

  static Result setParam(void *object, int parameter, const void *data)
  {
    return static_cast<InterfaceWrapper *>(object)->setParamImpl(parameter, data);
  }

  static size_t read(void *object, void *buffer, size_t length)
  {
    return static_cast<InterfaceWrapper *>(object)->readImpl(buffer, length);
  }

  static size_t write(void *object, const void *buffer, size_t length)
  {
    return static_cast<InterfaceWrapper *>(object)->writeImpl(buffer, length);
  }

private:
  Interface m_base;

  Interface * const m_interface;
  const Pin m_rx;
  const Pin m_tx;

  InterfaceWrapper(Interface *interface, PinNumber rx, PinNumber tx) :
    m_interface{interface},
    m_rx{pinInit(rx)},
    m_tx{pinInit(tx)}
  {
    assert(pinValid(m_rx));
    assert(pinValid(m_tx));

    pinOutput(m_rx, false);
    pinOutput(m_tx, false);
  }

  void setCallbackImpl(void (*callback)(void *), void *argument)
  {
    ifSetCallback(m_interface, callback, argument);
  }

  Result getParamImpl(int parameter, void *data)
  {
    return ifGetParam(m_interface, parameter, data);
  }

  Result setParamImpl(int parameter, const void *data)
  {
    switch (static_cast<IfParameter>(parameter))
    {
      case IF_RELEASE:
        pinReset(m_rx);
        pinReset(m_tx);
        break;

      default:
        break;
    }

    return ifSetParam(m_interface, parameter, data);
  }

  size_t readImpl(void *buffer, size_t length)
  {
    pinSet(m_rx);
    return ifRead(m_interface, buffer, length);
  }

  size_t writeImpl(const void *buffer, size_t length)
  {
    pinSet(m_tx);
    return ifWrite(m_interface, buffer, length);
  }
};

#endif // VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_INTERFACEWRAPPER_HPP_
