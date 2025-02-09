/*
 * Core/Shell/TerminalHelpers.hpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_TERMINALHELPERS_HPP_
#define VFS_SHELL_CORE_SHELL_TERMINALHELPERS_HPP_

#include <algorithm>
#include <cstring>
#include <cstdint>
#include <limits>
#include <type_traits>

class TerminalHelpers
{
public:
  enum class Color
  {
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,
    BRIGHT_BLACK,
    BRIGHT_RED,
    BRIGHT_GREEN,
    BRIGHT_YELLOW,
    BRIGHT_BLUE,
    BRIGHT_MAGENTA,
    BRIGHT_CYAN,
    BRIGHT_WHITE
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

  TerminalHelpers() = delete;
  TerminalHelpers(const TerminalHelpers &) = delete;
  TerminalHelpers &operator=(const TerminalHelpers &) = delete;

  template<typename T>
  static constexpr size_t serializedValueLength()
  {
    /*
     * Because digits10 is the number of decimal digits guaranteed to be correct, 3 more characters are needed to
     * display a value: one character for values with loss of precision, one for the minus sign and
     * one for the null character.
     */
    return std::numeric_limits<T>::digits10 + (std::is_signed_v<T> ? 1 : 0) + 2;
  }

  template<typename T>
  static typename std::enable_if_t<std::is_enum_v<T>, char *> int2str(char *buffer, T value,
      Width width = Width{0}, Format format = Format::DEC, Fill fill = Fill{' '})
  {
    return int2str(buffer, static_cast<std::underlying_type_t<T>>(value), width, format, fill);
  }

  template<typename T>
  static typename std::enable_if_t<!std::is_enum_v<T>, char *> int2str(char *buffer, T value,
      Width width = Width{0}, Format format = Format::DEC, Fill fill = Fill{' '})
  {
    if (format == Format::HEX)
      return int2hex(buffer, value, width.value, fill.value);
    else
      return int2dec(buffer, value, width.value, fill.value);
  }

  template<typename... Ts>
  static char *serialize(char *buffer, Ts... args)
  {
    return Serializer<Ts...>::serializeImpl(buffer, args...);
  }

  template<typename T>
  static T str2int(const char *buffer, size_t length, size_t *converted = nullptr)
  {
    const size_t chunkLength = std::min(serializedValueLength<T>() - 1, length);
    char nullTerminatedBuffer[serializedValueLength<T>()] = {};
    memcpy(nullTerminatedBuffer, buffer, chunkLength);

    char *lastConvertedCharacter;
    const T value = static_cast<T>(strtol(nullTerminatedBuffer, &lastConvertedCharacter, 0));

    if (converted != nullptr)
      *converted = lastConvertedCharacter - nullTerminatedBuffer;

    return value;
  }

private:
  template<typename... T>
  struct Serializer
  {
    static char *serializeImpl(char *buffer)
    {
      return buffer;
    }
  };

  template<typename T, typename... Ts>
  struct Serializer<T, Ts...>
  {
    static char *serializeImpl(char *buffer, T arg, Ts... args)
    {
      return Serializer<Ts...>::serializeImpl(int2str(buffer, arg), args...);
    }
  };

  template<typename... Ts>
  struct Serializer<const char *, Ts...>
  {
    static char *serializeImpl(char *buffer, const char *arg, Ts... args)
    {
      return Serializer<Ts...>::serializeImpl(append(buffer, arg), args...);
    }

  private:
    static char *append(char *buffer, const char *arg)
    {
      const size_t argLength = strlen(arg);
      memcpy(buffer, arg, argLength + 1);
      return buffer + argLength;
    }
  };

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

  static void reverse(char *first, char *last)
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

  template<typename T>
  static char *uint2dec(char *buffer, T value, size_t width, char fill)
  {
    char *output = buffer;

    do
    {
      const auto quotient = value / 10;
      const auto remainder = value - quotient * 10;

      *output++ = remainder + '0';
      value = quotient;
    }
    while (value);

    *output = '\0';
    reverse(buffer, output - 1);
    prepend(buffer, output - buffer, width, fill);

    return output;
  }

  template<typename T>
  static typename std::enable_if_t<std::is_signed_v<T>, char *> int2dec(char *buffer, T value, size_t width,
      char fill)
  {
    using UnsignedType = std::make_unsigned_t<T>;

    if (value < 0)
    {
      *buffer++ = '-';

      if (value != std::numeric_limits<T>::min())
        value = -value;

      if (width)
        --width;
    }

    return uint2dec<UnsignedType>(buffer, static_cast<UnsignedType>(value), width, fill);
  }

  template<typename T>
  static typename std::enable_if_t<std::is_unsigned_v<T>, char *> int2dec(char *buffer, T value, size_t width,
      char fill)
  {
    return uint2dec<T>(buffer, value, width, fill);
  }

  template<typename T>
  static char *int2hex(char *buffer, T value, size_t width, char fill)
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
    reverse(buffer, output - 1);
    prepend(buffer, output - buffer, width, fill);

    return output;
  }
};

#endif // VFS_SHELL_CORE_SHELL_TERMINALHELPERS_HPP_
