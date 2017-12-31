/*
 * MmfBuilder.cpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/linux/mmf.h>
#include "MmfBuilder.hpp"

Interface *MmfBuilder::build(const char *path)
{
  Interface * const interface = static_cast<Interface *>(init(MemoryMappedFile, path));

  if (interface == nullptr)
    return nullptr;

  MbrDescriptor mbr;

  if (mmfReadTable(interface, 0, 0, &mbr) == E_OK && mmfSetPartition(interface, &mbr) != E_OK)
  {
    deinit(interface);
    return nullptr;
  }

  return interface;
}
