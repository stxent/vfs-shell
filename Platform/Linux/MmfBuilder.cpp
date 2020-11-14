/*
 * MmfBuilder.cpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "MmfBuilder.hpp"
#include <halm/platform/generic/mmf.h>

Interface *MmfBuilder::build(const char *path)
{
  return static_cast<Interface *>(init(MemoryMappedFile, path));
}
