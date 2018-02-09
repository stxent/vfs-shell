/*
 * Core/Shell/Script.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPT_HPP_

#include <memory>
#include <tuple>
#include <utility>
#include <vector>
#include <xcore/error.h>
#include <xcore/fs.h>
#include "Shell/Environment.hpp"
#include "Shell/Terminal.hpp"
#include "Shell/TimeProvider.hpp"

struct ScriptEvent
{
  enum class Event
  {
    BUTTON_PRESSED,
    POINTER_MOVED,
    SERIAL_INPUT
  };

  Event event;
};

struct ButtonPressedEvent: public ScriptEvent
{
  unsigned short number;
  bool state;
};

struct PointerMovedEvent: public ScriptEvent
{
  short x;
  short y;
};

struct SerialInputEvent: public ScriptEvent
{
  size_t length;
};

class Script
{
public:
  typedef const char * const *ArgumentIterator;

  virtual ~Script() = default;

  virtual Result onEventReceived(const ScriptEvent *) = 0;

  virtual Environment &env() = 0;
  virtual FsHandle *fs() = 0;
  virtual Result run() = 0;
  virtual TimeProvider &time() = 0;
  virtual Terminal &tty() = 0;
};

class ScriptRunnerBase
{
public:
  virtual ~ScriptRunnerBase() = default;
  virtual const char *name() const = 0;
  virtual Result run(Script *parent, Script::ArgumentIterator, Script::ArgumentIterator) const = 0;
};

template<typename T, typename... ARGs>
class ScriptRunner: public ScriptRunnerBase
{
private:
  using Indices = std::make_index_sequence<sizeof...(ARGs)>;

public:
  ScriptRunner(ARGs... args) :
    m_args{std::make_tuple(args...)}
  {
  }

  virtual const char *name() const override
  {
    return T::name();
  }

  virtual Result run(Script *parent, Script::ArgumentIterator firstArgument,
      Script::ArgumentIterator lastArgument) const override
  {
    return impl(parent, firstArgument, lastArgument, Indices{});
  }

private:
  template<size_t... N>
  Result impl(Script *parent, Script::ArgumentIterator firstArgument, Script::ArgumentIterator lastArgument,
      std::index_sequence<N...>) const
  {
    return T{parent, firstArgument, lastArgument, std::get<N>(m_args)...}.run();
  }

  const std::tuple<ARGs...> m_args;
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPT_HPP_
