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
    case IF_RX_AVAILABLE:
      return "rx_available";
    case IF_RX_PENDING:
      return "rx_pending";
    case IF_RX_WATERMARK:
      return "rx_watermark";
    case IF_TX_AVAILABLE:
      return "tx_available";
    case IF_TX_PENDING:
      return "tx_pending";
    case IF_TX_WATERMARK:
      return "tx_watermark";
    case IF_ADDRESS:
      return "address";
    case IF_ADDRESS_64:
      return "address";
    case IF_RATE:
      return "rate";
    case IF_POSITION:
      return "position";
    case IF_POSITION_64:
      return "position";
    case IF_SIZE:
      return "size";
    case IF_SIZE_64:
      return "size";
    default:
      return "undefined";
  }
}

#endif // VFS_SHELL_CORE_SHELL_INTERFACES_INTERFACEPARAMETERS_HPP_
