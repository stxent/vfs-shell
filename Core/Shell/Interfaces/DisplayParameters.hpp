/*
 * Core/Shell/Interfaces/DisplayParameters.hpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_INTERFACES_DISPLAYPARAMETERS_HPP_
#define VFS_SHELL_CORE_SHELL_INTERFACES_DISPLAYPARAMETERS_HPP_

#include <dpm/displays/display.h>

template<DisplayParameter ID>
static constexpr const char *ifParamToName()
{
  switch (ID)
  {
    case IF_DISPLAY_ORIENTATION:
      return "orientation";
    case IF_DISPLAY_RESOLUTION:
      return "resolution";
    case IF_DISPLAY_WINDOW:
      return "window";
    default:
      return "undefined";
  }
}

#endif // VFS_SHELL_CORE_SHELL_INTERFACES_DISPLAYPARAMETERS_HPP_
