/*
 * Terminal.cpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <cstring>
#include "Shell/Terminal.hpp"

Terminal &operator<<(Terminal &output, short value)
{
  return Terminal::serialize(output, static_cast<int_fast16_t>(value));
}

Terminal &operator<<(Terminal &output, unsigned short value)
{
  return Terminal::serialize(output, static_cast<uint_fast16_t>(value));
}

Terminal &operator<<(Terminal &output, int value)
{
  return Terminal::serialize(output, static_cast<int_fast16_t>(value));
}

Terminal &operator<<(Terminal &output, unsigned int value)
{
  return Terminal::serialize(output, static_cast<uint_fast16_t>(value));
}

Terminal &operator<<(Terminal &output, long value)
{
  return Terminal::serialize(output, static_cast<int_fast32_t>(value));
}

Terminal &operator<<(Terminal &output, unsigned long value)
{
  return Terminal::serialize(output, static_cast<uint_fast32_t>(value));
}

Terminal &operator<<(Terminal &output, long long value)
{
  return Terminal::serialize(output, static_cast<int_fast64_t>(value));
}

Terminal &operator<<(Terminal &output, unsigned long long value)
{
  return Terminal::serialize(output, static_cast<uint_fast64_t>(value));
}

Terminal &operator<<(Terminal &output, const char *buffer)
{
  output.write(buffer, strlen(buffer));
  return output;
}

Terminal &operator<<(Terminal &output, char buffer)
{
  output.write(&buffer, sizeof(buffer));
  return output;
}

Terminal &operator<<(Terminal &output, Terminal::Control value)
{
  if (value == Terminal::Control::EOL)
    output.write("\r\n", 2);

  return output;
}

Terminal &operator<<(Terminal &output, Terminal::Fill value)
{
  output.setFill(value);
  return output;
}

Terminal &operator<<(Terminal &output, Terminal::Format value)
{
  output.setFormat(value);
  return output;
}

Terminal &operator<<(Terminal &output, Terminal::Width value)
{
  output.setWidth(value);
  return output;
}
