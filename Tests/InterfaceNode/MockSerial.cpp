/*
 * MockSerial.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "MockSerial.hpp"
#include <cppunit/extensions/HelperMacros.h>

static const InterfaceClass mockSerialTable = {
    sizeof(struct MockSerial), // size
    MockSerial::init,          // init
    MockSerial::deinit,        // deinit

    nullptr,                   // setCallback
    MockSerial::getParam,      // getParam
    MockSerial::setParam,      // setParam
    MockSerial::read,          // read
    MockSerial::write          // write
};

const InterfaceClass * const MockSerial = &mockSerialTable;

MockSerial::MockSerial()
{
  // m_base should be left untouched

  const auto result = byteQueueInit(&m_queue, QUEUE_SIZE);
  CPPUNIT_ASSERT(result == true);
}

MockSerial::~MockSerial()
{
  byteQueueDeinit(&m_queue);
}

Result MockSerial::getParamImpl(int parameter, void *data)
{
  switch (static_cast<SerialParameter>(parameter))
  {
    case IF_SERIAL_PARITY:
      *static_cast<SerialParity *>(data) = m_parity;
      return E_OK;

    case IF_SERIAL_CTS:
      *static_cast<bool *>(data) = m_cts;
      return E_OK;

    case IF_SERIAL_RTS:
      *static_cast<bool *>(data) = m_rts;
      return E_OK;

    case IF_SERIAL_DSR:
      *static_cast<bool *>(data) = m_dsr;
      return E_OK;

    case IF_SERIAL_DTR:
      *static_cast<bool *>(data) = m_dtr;
      return E_OK;

    default:
      return E_INVALID;
  }
}

Result MockSerial::setParamImpl(int parameter, const void *data)
{
  switch (static_cast<SerialParameter>(parameter))
  {
    case IF_SERIAL_PARITY:
      m_parity = *static_cast<const SerialParity *>(data);
      return E_OK;

    case IF_SERIAL_CTS:
      m_cts = *static_cast<const bool *>(data);
      return E_OK;

    case IF_SERIAL_RTS:
      m_rts = *static_cast<const bool *>(data);
      return E_OK;

    case IF_SERIAL_DSR:
      m_dsr = *static_cast<const bool *>(data);
      return E_OK;

    case IF_SERIAL_DTR:
      m_dtr = *static_cast<const bool *>(data);
      return E_OK;

    default:
      return E_INVALID;
  }
}

size_t MockSerial::readImpl(void *buffer, size_t length)
{
  return byteQueuePopArray(&m_queue, buffer, length);
}

size_t MockSerial::writeImpl(const void *buffer, size_t length)
{
  return byteQueuePushArray(&m_queue, buffer, length);
}
