/*
 * CardBuilder.cpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/generic/sdcard.h>
#include "CardBuilder.hpp"

Interface *CardBuilder::build(Interface *interface)
{
  const SdCardConfig config{interface, false};
  Interface * const card = static_cast<Interface *>(init(SdCard, &config));

  // TODO Partition table
  return card;
}
