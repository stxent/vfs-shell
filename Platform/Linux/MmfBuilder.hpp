/*
 * Platform/Linux/MmfBuilder.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_PLATFORM_LINUX_MMFBUILDER_HPP_
#define VFS_SHELL_PLATFORM_LINUX_MMFBUILDER_HPP_

#include <xcore/interface.h>

class MmfBuilder
{
public:
  MmfBuilder() = delete;
  MmfBuilder(const MmfBuilder &) = delete;
  MmfBuilder &operator=(const MmfBuilder &) = delete;

  static Interface *build(const char *);
};

#endif // VFS_SHELL_PLATFORM_LINUX_MMFBUILDER_HPP_
