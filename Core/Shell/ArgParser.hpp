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
    const char *key;
    const char *name;
    const char *info;
    size_t count;

    void (*callback)(void *, const char *);
  };

private:
  class OptionalArgumentFinder: std::unary_function<const Descriptor &, bool>
  {
  public:
    OptionalArgumentFinder(const char *name) :
      m_name{name}
    {
    }

    bool operator()(const Descriptor &object) const
    {
      return object.key != nullptr && strcmp(m_name, object.key) == 0;
    }

  private:
    const char *m_name;
  };

  struct PositionalArgumentFinder: std::unary_function<const Descriptor &, bool>
  {
    bool operator()(const Descriptor &object) const
    {
      return object.key == nullptr;
    }
  };

  template<typename DESC_ITER>
  static size_t computeNameLength(DESC_ITER iter)
  {
    return (iter->key != nullptr ? strlen(iter->key) : 0) + (iter->name != nullptr ? strlen(iter->name) : 0)
        + (iter->key != nullptr && iter->name != nullptr ? 1 : 0);
  }

  template<typename DESC_ITER>
  static size_t computeMaxOptionalNameLength(DESC_ITER firstDescriptor, DESC_ITER lastDescriptor)
  {
    size_t maxNameLength = 0;

    for (auto iter = firstDescriptor; iter != lastDescriptor; ++iter)
    {
      if (iter->key != nullptr)
        maxNameLength = std::max(maxNameLength, computeNameLength(iter));
    }

    return maxNameLength;
  }

  template<typename DESC_ITER>
  static size_t computeMaxPositionalNameLength(DESC_ITER firstDescriptor, DESC_ITER lastDescriptor)
  {
    auto findNext = [lastDescriptor](DESC_ITER startDescriptor){
        return std::find_if(startDescriptor, lastDescriptor, PositionalArgumentFinder{});
    };
    size_t maxNameLength = 0;

    for (auto iter = findNext(firstDescriptor); iter != lastDescriptor; iter = findNext(iter + 1))
    {
      if (iter->name != nullptr)
        maxNameLength = std::max(maxNameLength, computeNameLength(iter));
    }

    return maxNameLength;
  }

  static void printTrailingSpaces(Terminal &tty, size_t count)
  {
    for (size_t i = 0; i < count; ++i)
      tty << ' ';
  }

public:
  ArgParser() = delete;
  ArgParser(const ArgParser &) = delete;
  ArgParser &operator=(const ArgParser &) = delete;

  template<typename DESC_ITER>
  static void help(Terminal &tty, const char *name, DESC_ITER firstDescriptor, DESC_ITER lastDescriptor)
  {
    const size_t maxOptionalNameLength = computeMaxOptionalNameLength(firstDescriptor, lastDescriptor);
    const size_t maxPositionalNameLength = computeMaxPositionalNameLength(firstDescriptor, lastDescriptor);

    // Print usage info
    if (name != nullptr)
    {
      tty << Terminal::BOLD << "Usage: " << Terminal::REGULAR << name;

      // Print brief information about optional arguments
      for (auto iter = firstDescriptor; iter != lastDescriptor; ++iter)
      {
        if (iter->key != nullptr)
        {
          tty << " [" << iter->key;
          if (iter->count && iter->name != nullptr)
            tty << ' ' << iter->name;
          tty << ']';
        }
      }

      // Print brief information about positional arguments
      if (maxPositionalNameLength > 0)
      {
        auto findNext = [lastDescriptor](DESC_ITER startDescriptor){
            return std::find_if(startDescriptor, lastDescriptor, PositionalArgumentFinder{});
        };

        for (auto iter = findNext(firstDescriptor); iter != lastDescriptor; iter = findNext(iter + 1))
        {
          if (iter->name != nullptr)
          {
            tty << " [" << iter->name << ']';
            if (iter->count != 1)
              tty << "...";
          }
        }
      }

      tty << Terminal::EOL << Terminal::EOL;
    }

    // Describe positional arguments
    if (maxPositionalNameLength > 0)
    {
      tty << Terminal::BOLD << "Positional arguments:" << Terminal::REGULAR << Terminal::EOL;

      auto findNext = [lastDescriptor](DESC_ITER startDescriptor){
          return std::find_if(startDescriptor, lastDescriptor, PositionalArgumentFinder{});
      };

      for (auto iter = findNext(firstDescriptor); iter != lastDescriptor; iter = findNext(iter + 1))
      {
        if (iter->name != nullptr && iter->info != nullptr)
        {
          tty << "  " << iter->name;
          printTrailingSpaces(tty, maxPositionalNameLength - computeNameLength(iter));
          tty << "  " << iter->info << Terminal::EOL;
        }
      }

      tty << Terminal::EOL;
    }

    // Describe optional arguments
    if (maxOptionalNameLength > 0)
    {
      tty << Terminal::BOLD << "Optional arguments:" << Terminal::REGULAR << Terminal::EOL;

      for (auto iter = firstDescriptor; iter != lastDescriptor; ++iter)
      {
        if (iter->key != nullptr)
        {
          tty << "  " << Terminal::BOLD << iter->key << Terminal::REGULAR;
          if (iter->name != nullptr)
            tty << " " << iter->name;
          printTrailingSpaces(tty, maxOptionalNameLength - computeNameLength(iter));
          tty << "  " << iter->info << Terminal::EOL;
        }
      }
    }
  }

  template<typename ARG_ITER, typename DESC_ITER>
  static void invoke(ARG_ITER firstArgument, ARG_ITER lastArgument, DESC_ITER firstDescriptor,
      DESC_ITER lastDescriptor, std::function<void(const char *)> callback)
  {
    for (auto currentArgument = firstArgument; currentArgument != lastArgument;)
    {
      const auto optional = std::find_if(firstDescriptor, lastDescriptor, OptionalArgumentFinder{*currentArgument});

      if (optional != lastDescriptor)
      {
        const size_t argumentsLeft = static_cast<size_t>(lastArgument - currentArgument);

        if (argumentsLeft >= optional->count)
          currentArgument += 1 + optional->count;
        else
          break;
      }
      else
      {
        callback(*currentArgument);
        ++currentArgument;
      }
    }
  }

  template<typename CONTAINER, typename ARG_ITER, typename DESC_ITER>
  static CONTAINER parse(ARG_ITER firstArgument, ARG_ITER lastArgument,
      DESC_ITER firstDescriptor, DESC_ITER lastDescriptor, bool *status = nullptr)
  {
    CONTAINER container;
    parseImpl(firstArgument, lastArgument, firstDescriptor, lastDescriptor, &container, status);
    return container;
  }

private:
  template<typename ARG_ITER, typename DESC_ITER>
  static void parseImpl(ARG_ITER firstArgument, ARG_ITER lastArgument, DESC_ITER firstDescriptor,
      DESC_ITER lastDescriptor, void *container, bool *status)
  {
    size_t positionalCounter = 0;
    bool parsingStatus = true;

    auto findNextPositional = [lastDescriptor](DESC_ITER startDescriptor){
        return std::find_if(startDescriptor, lastDescriptor, PositionalArgumentFinder{});
    };
    auto positional = findNextPositional(firstDescriptor);

    for (auto currentArgument = firstArgument; currentArgument != lastArgument;)
    {
      const auto optional = std::find_if(firstDescriptor, lastDescriptor, OptionalArgumentFinder{*currentArgument});

      if (optional != lastDescriptor)
      {
        const size_t left = static_cast<size_t>(lastArgument - currentArgument);

        if (left >= optional->count + 1)
        {
          if (optional->callback != nullptr)
          {
            if (optional->count > 0)
            {
              for (size_t i = 1; i <= optional->count; ++i)
                optional->callback(container, *(currentArgument + i));
            }
            else
              optional->callback(container, nullptr);
          }

          currentArgument += 1 + optional->count;
        }
        else
        {
          // The optional argument descriptor requires more arguments than available
          parsingStatus = false;
          break;
        }
      }
      else if (positional != lastDescriptor)
      {
        if (positional->callback != nullptr)
          positional->callback(container, *currentArgument);
        ++currentArgument;

        if (++positionalCounter == positional->count)
        {
          positional = findNextPositional(positional + 1);
          positionalCounter = 0;
        }
      }
      else
      {
        // Unknown argument
        parsingStatus = false;
        break;
      }
    }

    if (positional != lastDescriptor && positional->count > 0)
    {
      // Not all positional arguments were found
      parsingStatus = false;
    }

    if (status != nullptr)
      *status = parsingStatus;
  }
};

#endif // VFS_SHELL_CORE_SHELL_ARGPARSER_HPP_
