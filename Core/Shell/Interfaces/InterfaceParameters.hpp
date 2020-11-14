/*
 * Core/Shell/Interfaces/InterfaceParameters.hpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_INTERFACES_INTERFACEPARAMETERS_HPP_
#define VFS_SHELL_CORE_SHELL_INTERFACES_INTERFACEPARAMETERS_HPP_

#include <xcore/interface.h>

template<IfParameter ID>
static constexpr const char *ifParamToName()
{
  switch (ID)
  {
    case IF_AVAILABLE:
      return "available";
    case IF_PENDING:
      return "pending";
    case IF_ADDRESS:
      return "address";
    case IF_PRIORITY:
      return "priority";
    case IF_RATE:
      return "rate";
    case IF_ALIGNMENT:
      return "alignment";
    case IF_POSITION:
      return "position";
    case IF_POSITION_64:
      return "position";
    case IF_SIZE:
      return "size";
    case IF_SIZE_64:
      return "size";
    case IF_WIDTH:
      return "width";
    case IF_STATUS:
      return "status";
    default:
      return "undefined";
  }
}

#endif // VFS_SHELL_CORE_SHELL_INTERFACES_INTERFACEPARAMETERS_HPP_
