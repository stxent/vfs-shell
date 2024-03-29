/*
 * Terminal.cpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Shell/Terminal.hpp"
#include <cstring>

Terminal &operator<<(Terminal &output, short value)
{
  return Terminal::serialize(output, value);
}

Terminal &operator<<(Terminal &output, unsigned short value)
{
  return Terminal::serialize(output, value);
}

Terminal &operator<<(Terminal &output, int value)
{
  return Terminal::serialize(output, value);
}

Terminal &operator<<(Terminal &output, unsigned int value)
{
  return Terminal::serialize(output, value);
}

Terminal &operator<<(Terminal &output, long value)
{
  return Terminal::serialize(output, value);
}

Terminal &operator<<(Terminal &output, unsigned long value)
{
  return Terminal::serialize(output, value);
}

Terminal &operator<<(Terminal &output, long long value)
{
  return Terminal::serialize(output, value);
}

Terminal &operator<<(Terminal &output, unsigned long long value)
{
  return Terminal::serialize(output, value);
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

Terminal &operator<<(Terminal &output, Terminal::Color value)
{
  if (output.m_coloration)
  {
    const char high = value >= Terminal::Color::BRIGHT_BLACK ? '9' : '3';
    char low = static_cast<char>('0' + static_cast<int>(value));

    if (value >= Terminal::Color::BRIGHT_BLACK)
      low -= static_cast<int>(Terminal::Color::BRIGHT_BLACK);

    const char buffer[] = {'\033', '[', high, low, 'm'};
    output.write(buffer, sizeof(buffer));
  }

  return output;
}

Terminal &operator<<(Terminal &output, Terminal::Control value)
{
  switch (value)
  {
    case Terminal::EOL:
      output.write("\r\n", 2);
      break;

    case Terminal::RESET:
      if (output.m_coloration)
        output.write("\033[0m", 4);
      break;

    case Terminal::BOLD:
      if (output.m_coloration)
        output.write("\033[1m", 4);
      break;
  }

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
