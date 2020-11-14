/*
 * CardBuilder.cpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "CardBuilder.hpp"
#include <halm/generic/mmcsd.h>

Interface *CardBuilder::build(Interface *interface)
{
  const MMCSDConfig config{interface, false};
  Interface * const card = static_cast<Interface *>(init(MMCSD, &config));

  // TODO Partition table
  return card;
}
