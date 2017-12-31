/*
 * Core/Shell/Terminal.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_TERMINAL_HPP_
#define VFS_SHELL_CORE_SHELL_TERMINAL_HPP_

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <type_traits>

class Script;

class Terminal
{
public:
  enum Control
  {
    EOL
  };

  enum class Format
  {
    DEC,
    HEX
  };

  struct Fill
  {
    char value;
  };

  struct Width
  {
    size_t value;
  };

  virtual ~Terminal() = default;
  virtual void subscribe(Script *) = 0;
  virtual void unsubscribe(const Script *) = 0;
  virtual size_t read(char *, size_t) = 0;
  virtual size_t write(const char *, size_t) = 0;

  Terminal() :
    m_fill{' '},
    m_format{Format::DEC},
    m_width{1}
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
  friend Terminal &operator<<(Terminal &, Terminal::Control);
  friend Terminal &operator<<(Terminal &, Terminal::Fill);
  friend Terminal &operator<<(Terminal &, Terminal::Format);
  friend Terminal &operator<<(Terminal &, Terminal::Width);

  Fill m_fill;
  Format m_format;
  Width m_width;

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

  static void invert(char *first, char *last)
  {
    while (first < last)
    {
      const char tmp = *first;
      *first = *last;
      *last = tmp;

      first++;
      last--;
    }
  }

  static void prepend(char *first, size_t length, size_t width, char fill)
  {
    if (length < width)
    {
      size_t offset = width - length;
      ++length;

      while (length--)
        first[length + offset] = first[length];
      while (offset--)
        first[offset] = fill;
    }
  }

  template<class T>
  static void uint2dec(char *buffer, T value, size_t width, char fill)
  {
    char *output = buffer;

    do
    {
      const unsigned int remainder = static_cast<unsigned int>(value % 10);
      *output++ = remainder + '0';
      value /= 10;
    }
    while (value);

    *output = '\0';
    invert(buffer, output - 1);
    prepend(buffer, output - buffer, width, fill);
  }

  template<class T>
  static typename std::enable_if<std::is_signed<T>::value>::type int2dec(char *buffer, T value, size_t width,
      char fill)
  {
    char * const output = value < 0 ? buffer + 1 : buffer;

    uint2dec<T>(output, abs(value), width, fill);
    if (value < 0)
      *buffer = '-';
  }

  template<class T>
  static typename std::enable_if<std::is_unsigned<T>::value>::type int2dec(char *buffer, T value, size_t width,
      char fill)
  {
    uint2dec<T>(buffer, value, width, fill);
  }

  template<class T>
  static void int2hex(char *buffer, T value, size_t width, char fill)
  {
    char *output = buffer;

    do
    {
      const uint8_t part = static_cast<uint8_t>(value) & 0x0F;
      *output++ = part < 10 ? part + '0' : part - 10 + 'A';
      value >>= 4;
    }
    while (value);

    *output = '\0';
    invert(buffer, output - 1);
    prepend(buffer, output - buffer, width, fill);
  }

  template<class T>
  static void int2str(char *buffer, T value, Width width, Format format, Fill fill)
  {
    if (format == Format::HEX)
      int2hex(buffer, value, width.value, fill.value);
    else
      int2dec(buffer, value, width.value, fill.value);
  }

  template<class T>
  static Terminal &serialize(Terminal &output, T value)
  {
    char buffer[24];

    Terminal::int2str(buffer, value, output.m_width, output.m_format, output.m_fill);
    output.write(buffer, strlen(buffer));
    return output;
  }
};

#endif // VFS_SHELL_CORE_SHELL_TERMINAL_HPP_
