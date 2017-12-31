/*
 * Core/Shell/ScriptHeaders.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTHEADERS_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTHEADERS_HPP_

#include <cstddef>

struct ScriptHeaders
{
  ScriptHeaders() = delete;
  ScriptHeaders(const ScriptHeaders &) = delete;
  ScriptHeaders &operator=(const ScriptHeaders &) = delete;

  static constexpr size_t MAX_HEADER_SIZE = 4;

  static constexpr size_t OBJECT_HEADER_SIZE = 4;
  static const char OBJECT_HEADER[OBJECT_HEADER_SIZE];
  static constexpr size_t SCRIPT_HEADER_SIZE = 2;
  static const char SCRIPT_HEADER[SCRIPT_HEADER_SIZE];
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTHEADERS_HPP_
