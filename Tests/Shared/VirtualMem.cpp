/*
 * VirtualMem.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "VirtualMem.hpp"
#include <cppunit/extensions/HelperMacros.h>
#include <cstring>

static const InterfaceClass virtualMemTable = {
    sizeof(struct VirtualMem), // size
    VirtualMem::init,          // init
    VirtualMem::deinit,        // deinit

    nullptr,                   // setCallback
    VirtualMem::getParam,      // getParam
    VirtualMem::setParam,      // setParam
    VirtualMem::read,          // read
    VirtualMem::write          // write
};

const InterfaceClass * const VirtualMem = &virtualMemTable;

VirtualMem::VirtualMem(size_t size) :
  m_size{size}
{
  // m_base should be left untouched

  if (m_size)
  {
    m_data = std::make_unique<uint8_t []>(m_size);
    std::fill(m_data.get(), m_data.get() + m_size, 0);
  }
}

VirtualMem::~VirtualMem()
{
}

Result VirtualMem::getParamImpl(int parameter, void *data)
{
  switch (static_cast<IfParameter>(parameter))
  {
    case IF_POSITION_64:
      *static_cast<uint64_t *>(data) = static_cast<uint64_t>(m_position);
      return E_OK;

    case IF_SIZE_64:
      *static_cast<uint64_t *>(data) = static_cast<uint64_t>(m_size);
      return E_OK;

    case IF_STATUS:
      return E_OK;

    default:
      return E_INVALID;
  }
}

Result VirtualMem::setParamImpl(int parameter, const void *data)
{
  switch (static_cast<IfParameter>(parameter))
  {
    case IF_POSITION_64:
    {
      const size_t position = static_cast<size_t>(*static_cast<const uint64_t *>(data));

      if (position < m_size)
      {
        m_position = position;
        return E_OK;
      }
      else
        return E_ADDRESS;
    }

    case IF_ACQUIRE:
      return E_OK;

    case IF_RELEASE:
      return E_OK;

    default:
      return E_INVALID;
  }
}

size_t VirtualMem::readImpl(void *buffer, size_t length)
{
  memcpy(buffer, m_data.get() + m_position, length);
  m_position += length;

  return length;
}

size_t VirtualMem::writeImpl(const void *buffer, size_t length)
{
  memcpy(m_data.get() + m_position, buffer, length);
  m_position += length;

  return length;
}
