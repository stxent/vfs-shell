/*
 * MockInterface.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "MockInterface.hpp"
#include <cppunit/extensions/HelperMacros.h>

static const InterfaceClass mockInterfaceTable = {
    sizeof(struct MockInterface), // size
    MockInterface::init,          // init
    MockInterface::deinit,        // deinit

    nullptr,                      // setCallback
    MockInterface::getParam,      // getParam
    MockInterface::setParam,      // setParam
    MockInterface::read,          // read
    MockInterface::write          // write
};

const InterfaceClass * const MockInterface = &mockInterfaceTable;

MockInterface::MockInterface()
{
  // m_base should be left untouched

  const auto result = byteQueueInit(&m_queue, QUEUE_SIZE);
  CPPUNIT_ASSERT(result == true);
}

MockInterface::~MockInterface()
{
  byteQueueDeinit(&m_queue);
}

Result MockInterface::getParamImpl(int parameter, void *data)
{
  switch (static_cast<IfParameter>(parameter))
  {
    case IF_RX_AVAILABLE:
      *static_cast<size_t *>(data) = m_rxAvailable;
      return E_OK;

    case IF_RX_PENDING:
      *static_cast<size_t *>(data) = m_rxPending;
      return E_OK;

    case IF_RX_WATERMARK:
      *static_cast<size_t *>(data) = m_rxWatermark;
      return E_OK;

    case IF_TX_AVAILABLE:
      *static_cast<size_t *>(data) = m_txAvailable;
      return E_OK;

    case IF_TX_PENDING:
      *static_cast<size_t *>(data) = m_txPending;
      return E_OK;

    case IF_TX_WATERMARK:
      *static_cast<size_t *>(data) = m_txWatermark;
      return E_OK;

    case IF_ADDRESS:
      *static_cast<uint32_t *>(data) = static_cast<uint32_t>(m_address);
      return E_OK;

    case IF_ADDRESS_64:
      *static_cast<uint64_t *>(data) = m_address;
      return E_OK;

    case IF_RATE:
      *static_cast<uint32_t *>(data) = m_rate;
      return E_OK;

    case IF_POSITION:
      *static_cast<uint32_t *>(data) = static_cast<uint32_t>(m_position);
      return E_OK;

    case IF_POSITION_64:
      *static_cast<uint64_t *>(data) = m_position;
      return E_OK;

    case IF_SIZE:
      *static_cast<uint32_t *>(data) = static_cast<uint32_t>(m_size);
      return E_OK;

    case IF_SIZE_64:
      *static_cast<uint64_t *>(data) = m_size;
      return E_OK;

    default:
      return E_INVALID;
  }
}

Result MockInterface::setParamImpl(int parameter, const void *data)
{
  switch (static_cast<IfParameter>(parameter))
  {
    case IF_RX_AVAILABLE:
      m_rxAvailable = *static_cast<const size_t *>(data);
      return E_OK;

    case IF_RX_PENDING:
      m_rxPending = *static_cast<const size_t *>(data);
      return E_OK;

    case IF_RX_WATERMARK:
      m_rxWatermark = *static_cast<const size_t *>(data);
      return E_OK;

    case IF_TX_AVAILABLE:
      m_txAvailable = *static_cast<const size_t *>(data);
      return E_OK;

    case IF_TX_PENDING:
      m_txPending = *static_cast<const size_t *>(data);
      return E_OK;

    case IF_TX_WATERMARK:
      m_txWatermark = *static_cast<const size_t *>(data);
      return E_OK;

    case IF_ADDRESS:
      m_address = *static_cast<const uint32_t *>(data);
      return E_OK;

    case IF_ADDRESS_64:
      m_address = *static_cast<const uint64_t *>(data);
      return E_OK;

    case IF_RATE:
      m_rate = *static_cast<const uint32_t *>(data);
      return E_OK;

    case IF_POSITION:
      m_position = *static_cast<const uint32_t *>(data);
      return E_OK;

    case IF_POSITION_64:
      m_position = *static_cast<const uint64_t *>(data);
      return E_OK;

    case IF_SIZE:
      m_size = *static_cast<const uint32_t *>(data);
      return E_OK;

    case IF_SIZE_64:
      m_size = *static_cast<const uint64_t *>(data);
      return E_OK;

    default:
      return E_INVALID;
  }
}

size_t MockInterface::readImpl(void *buffer, size_t length)
{
  return byteQueuePopArray(&m_queue, buffer, length);
}

size_t MockInterface::writeImpl(const void *buffer, size_t length)
{
  return byteQueuePushArray(&m_queue, buffer, length);
}
