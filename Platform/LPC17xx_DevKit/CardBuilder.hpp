/*
 * Platform/LPC17xx_DevKit/CardBuilder.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_CARDBUILDER_HPP_
#define VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_CARDBUILDER_HPP_

#include <xcore/interface.h>

class CardBuilder
{
public:
  CardBuilder() = delete;
  CardBuilder(const CardBuilder &) = delete;
  CardBuilder &operator=(const CardBuilder &) = delete;

  static Interface *build(Interface *);
};

#endif // VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_CARDBUILDER_HPP_
