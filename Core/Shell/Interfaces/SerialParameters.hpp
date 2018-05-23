/*
 * Core/Shell/Interfaces/SerialParameters.hpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_INTERFACES_SERIALPARAMETERS_HPP_
#define VFS_SHELL_CORE_SHELL_INTERFACES_SERIALPARAMETERS_HPP_

#include <halm/generic/serial.h>

template<SerialParameter ID>
static constexpr const char *ifParamToName()
{
  switch (ID)
  {
    case IF_SERIAL_PARITY:
      return "parity";
    case IF_SERIAL_CTS:
      return "cts";
    case IF_SERIAL_RTS:
      return "rts";
    case IF_SERIAL_DSR:
      return "dsr";
    case IF_SERIAL_DTR:
      return "dtr";
    default:
      return "undefined";
  }
}

#endif // VFS_SHELL_CORE_SHELL_INTERFACES_SERIALPARAMETERS_HPP_
