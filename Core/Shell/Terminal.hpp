/*
 * Core/Shell/Terminal.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_TERMINAL_HPP_
#define VFS_SHELL_CORE_SHELL_TERMINAL_HPP_

#include "Shell/TerminalHelpers.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <type_traits>

class Script;

class Terminal
{
public:
  using Color = TerminalHelpers::Color;
  using Format = TerminalHelpers::Format;
  using Fill = TerminalHelpers::Fill;
  using Width = TerminalHelpers::Width;

  enum Control
  {
    EOL,
    RESET,
    BOLD
  };

  virtual ~Terminal() = default;
  virtual void subscribe(Script *) = 0;
  virtual void unsubscribe(const Script *) = 0;
  virtual size_t read(char *, size_t) = 0;
  virtual size_t write(const char *, size_t) = 0;

  Terminal(bool coloration = false) :
    m_fill{' '},
    m_format{Format::DEC},
    m_width{1},
    m_coloration{coloration}
  {
  }

  Fill fill() const
  {
    return m_fill;
  }

  Format format() const
  {
    return m_format;
  }

  Width width() const
  {
    return m_width;
  }

private:
  friend Terminal &operator<<(Terminal &, short);
  friend Terminal &operator<<(Terminal &, unsigned short);
  friend Terminal &operator<<(Terminal &, int);
  friend Terminal &operator<<(Terminal &, unsigned int);
  friend Terminal &operator<<(Terminal &, long);
  friend Terminal &operator<<(Terminal &, unsigned long);
  friend Terminal &operator<<(Terminal &, long long);
  friend Terminal &operator<<(Terminal &, unsigned long long);

  friend Terminal &operator<<(Terminal &, const char *);
  friend Terminal &operator<<(Terminal &, char);
  friend Terminal &operator<<(Terminal &, Terminal::Color);
  friend Terminal &operator<<(Terminal &, Terminal::Control);
  friend Terminal &operator<<(Terminal &, Terminal::Fill);
  friend Terminal &operator<<(Terminal &, Terminal::Format);
  friend Terminal &operator<<(Terminal &, Terminal::Width);

  Fill m_fill;
  Format m_format;
  Width m_width;
  bool m_coloration;

  void setFill(Fill value)
  {
    m_fill = value;
  }

  void setFormat(Format value)
  {
    m_format = value;
  }

  void setWidth(Width value)
  {
    m_width = value;
  }

  template<typename T>
  static Terminal &serialize(Terminal &output, T value)
  {
    char buffer[TerminalHelpers::serializedValueLength<T>()];

    TerminalHelpers::int2str(buffer, value, output.m_width, output.m_format, output.m_fill);
    output.write(buffer, strlen(buffer));
    return output;
  }
};

#endif // VFS_SHELL_CORE_SHELL_TERMINAL_HPP_
