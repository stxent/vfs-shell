/*
 * Core/Shell/ArgParser.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_ARGPARSER_HPP_
#define VFS_SHELL_CORE_SHELL_ARGPARSER_HPP_

#include <algorithm>
#include <cstddef>
#include <functional>
#include "Shell/Script.hpp"
#include "Shell/Terminal.hpp"

class ArgParser
{
public:
  struct Descriptor
  {
    const char *name;
    const char *info;
    ssize_t count;

    void (*callback)(void *, const char *);
    void *argument;
  };

  ArgParser() = delete;
  ArgParser(const ArgParser &) = delete;
  ArgParser &operator=(const ArgParser &) = delete;

  template<class T>
  static void help(Terminal &tty, T firstDescriptor, T lastDescriptor)
  {
    for (auto desc = firstDescriptor; desc != lastDescriptor; ++desc)
    {
      if (desc->name != nullptr)
        tty << "\t" << desc->name << ": " << desc->info << Terminal::EOL;
    }
  }

  template<class T>
  static void invoke(Script::ArgumentIterator firstArgument, Script::ArgumentIterator lastArgument,
      T firstDescriptor, T lastDescriptor, std::function<void(const char *)> callback)
  {
    for (auto pos = firstArgument; pos != lastArgument; ++pos)
    {
      bool found = false;

      for (auto desc = firstDescriptor; desc != lastDescriptor; ++desc)
      {
        if (desc->name == nullptr)
          continue;

        if (!strcmp(*pos, desc->name))
        {
          pos += desc->count;
          found = true;
          break;
        }
      }

      if (!found)
        callback(*pos);
    }
  }

  template<class T>
  static void parse(Script::ArgumentIterator firstArgument, Script::ArgumentIterator lastArgument,
      T firstDescriptor, T lastDescriptor)
  {
    struct PositionalArgumentFinder: std::unary_function<const Descriptor &, bool>
    {
      bool operator()(const Descriptor &object) const
          {
        return object.name == nullptr;
      }
    };

    const auto positional = std::find_if(firstDescriptor, lastDescriptor, PositionalArgumentFinder {});

    for (auto pos = firstArgument; pos != lastArgument; ++pos)
    {
      bool found = false;

      for (auto desc = firstDescriptor; desc != lastDescriptor; ++desc)
      {
        if (desc->name == nullptr)
          continue;

        if (!strcmp(*pos, desc->name) && lastArgument - pos >= desc->count)
        {
          found = true;
          desc->callback(desc->argument, desc->count ? *(pos + 1) : nullptr);
          break;
        }
      }

      if (!found && positional != lastDescriptor)
        positional->callback(positional->argument, *pos);
    }
  }
};

#endif // VFS_SHELL_CORE_SHELL_ARGPARSER_HPP_
