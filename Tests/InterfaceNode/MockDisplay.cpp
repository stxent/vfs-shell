/*
 * MockDisplay.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "MockDisplay.hpp"
#include <algorithm>
#include <cppunit/extensions/HelperMacros.h>
#include <cstring>

static const InterfaceClass mockDisplayTable = {
    sizeof(struct MockDisplay), // size
    MockDisplay::init,          // init
    MockDisplay::deinit,        // deinit

    nullptr,                    // setCallback
    MockDisplay::getParam,      // getParam
    MockDisplay::setParam,      // setParam
    MockDisplay::read,          // read
    MockDisplay::write          // write
};

const InterfaceClass * const MockDisplay = &mockDisplayTable;

MockDisplay::MockDisplay()
{
  // m_base should be left untouched

  std::fill(m_data, m_data + sizeof(m_data), static_cast<uint8_t>(' '));
}

MockDisplay::~MockDisplay()
{
}

Result MockDisplay::getParamImpl(int parameter, void *data)
{
  switch (static_cast<DisplayParameter>(parameter))
  {
    case IF_DISPLAY_ORIENTATION:
      memcpy(data, &m_orientation, sizeof(m_orientation));
      return E_OK;

    case IF_DISPLAY_RESOLUTION:
      memcpy(data, &m_resolution, sizeof(m_resolution));
      return E_OK;

    case IF_DISPLAY_WINDOW:
      memcpy(data, &m_window, sizeof(m_window));
      return E_OK;

    default:
      break;
  }

  switch (static_cast<IfParameter>(parameter))
  {
    case IF_SIZE:
      *static_cast<uint32_t *>(data) = static_cast<uint32_t>(sizeof(m_data));
      return E_OK;

    default:
      return E_INVALID;
  }
}

Result MockDisplay::setParamImpl(int parameter, const void *data)
{
  switch (static_cast<DisplayParameter>(parameter))
  {
    case IF_DISPLAY_ORIENTATION:
      memcpy(&m_orientation, data, sizeof(m_orientation));
      return E_OK;

    case IF_DISPLAY_RESOLUTION:
      memcpy(&m_resolution, data, sizeof(m_resolution));
      return E_OK;

    case IF_DISPLAY_WINDOW:
      memcpy(&m_window, data, sizeof(m_window));
      return E_OK;

    default:
      break;
  }

  switch (static_cast<IfParameter>(parameter))
  {
    case IF_POSITION:
    {
      const auto position = *static_cast<const uint32_t *>(data);

      if (position <= static_cast<uint32_t>(sizeof(m_data)))
      {
        m_position = static_cast<size_t>(position);
        return E_OK;
      }
      else
        return E_ADDRESS;
    }

    default:
      return E_INVALID;
  }
}

size_t MockDisplay::readImpl(void *buffer, size_t length)
{
  const auto end = std::min(m_position + length, sizeof(m_data));
  const auto count = end - m_position;

  if (count > 0 && end <= sizeof(m_data))
  {
    std::copy(m_data + m_position, m_data + end, static_cast<uint8_t *>(buffer));
    m_position = end;
    return count;
  }
  else
    return 0;
}

size_t MockDisplay::writeImpl(const void *buffer, size_t length)
{
  const auto end = std::min(m_position + length, sizeof(m_data));
  const auto count = end - m_position;

  if (count > 0 && end <= sizeof(m_data))
  {
    std::copy_n(static_cast<const uint8_t *>(buffer), count, m_data + m_position);
    m_position = end;
    return count;
  }
  else
    return 0;
}
