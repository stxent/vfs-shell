/*
 * LineParser.cpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <algorithm>
#include "Shell/LineParser.hpp"
#include "Shell/Terminal.hpp"

LineParser::LineParser(Terminal &terminal, bool echo) :
  m_terminal{terminal},
  m_cursor{0},
  m_length{0},
  m_carriage{0},
  m_echo{echo}
{
}

bool LineParser::eraseNext()
{
  if (m_cursor < m_length)
  {
    --m_length;

    if (m_cursor < m_length)
    {
      // Remove character from the buffer
      std::move(m_command.begin() + m_cursor + 1, m_command.begin() + m_length + 1, m_command.begin() + m_cursor);

      if (m_echo)
      {
        const size_t remaining = m_length - m_cursor;

        if (remaining > 0)
        {
          // Clear to end of line
          m_terminal.write("\x1B[K", 3);
          m_terminal.write(m_command.begin() + m_cursor, remaining);
          rewindCursor(remaining);
        }
      }
    }
    else
    {
      if (m_echo)
        m_terminal.write(" \b", 2);
    }

    return true;
  }
  else
    return false;
}

bool LineParser::erasePrevious()
{
  if (m_cursor > 0)
  {
    if (m_cursor < m_length)
    {
      // Remove character from the buffer
      std::move(m_command.begin() + m_cursor, m_command.begin() + m_length, m_command.begin() + m_cursor - 1);

      if (m_echo)
      {
        const size_t remaining = m_length - m_cursor;

        // Move left and clear to end of line
        m_terminal.write("\b\x1B[K", 4);
        m_terminal.write(m_command.begin() + m_cursor - 1, remaining);
        rewindCursor(remaining);
      }
    }
    else
    {
      if (m_echo)
        m_terminal.write("\b \b", 3);
    }

    --m_cursor;
    --m_length;

    return true;
  }
  else
    return false;
}

bool LineParser::insert(char c)
{
  if (m_length < m_command.size() - 1)
  {
    if (m_cursor < m_length)
    {
      // Insert the character in the midst of the string
      std::move(m_command.begin() + m_cursor, m_command.begin() + m_length, m_command.begin() + m_cursor + 1);
      // Replace character in the buffer by the new one
      m_command[m_cursor] = c;

      if (m_echo)
      {
        // Print the character
        m_terminal.write(&c, 1);
        // Print remaining characters
        m_terminal.write(m_command.begin() + m_cursor + 1, m_length - m_cursor);
        // Rewind to current position
        rewindCursor(m_length - m_cursor);
      }
    }
    else
    {
      // Append the character to the end of the string
      m_command[m_length] = c;

      if (m_echo)
        m_terminal.write(&c, 1);
    }

    ++m_cursor;
    ++m_length;

    return true;
  }
  else
    return false;
}

void LineParser::moveCursorLeft()
{
  if (m_cursor > 0)
  {
    --m_cursor;

    if (m_echo)
      m_terminal << "\x1B[D";
  }
}

void LineParser::moveCursorRight()
{
  if (m_cursor < m_length)
  {
    ++m_cursor;

    if (m_echo)
      m_terminal << "\x1B[C";
  }
}

LineParser::Status LineParser::parse(char c)
{
  Status status = Status::CONSUMED;

  if (c == '\x03') // End of text
  {
    status = Status::TERMINATED;
    m_command[m_length] = '\0';

    if (m_echo)
      m_terminal << "^C" << Terminal::EOL;
  }
  else if (c == '\r' || c == '\n') // New line
  {
    if (!m_carriage || m_carriage == c)
    {
      status = Status::COMPLETED;
      m_command[m_length] = '\0';

      if (m_echo)
        m_terminal << Terminal::EOL;
    }
    m_carriage = c;
  }
  else if (c == '\b' || c == '\x7F') // Backspace
  {
    if (!erasePrevious())
      status = Status::DISCARDED;
    m_carriage = 0;
  }
  else
  {
    if (!insert(c))
      status = Status::DISCARDED;
    m_carriage = 0;
  }

  return status;
}

void LineParser::reset()
{
  m_cursor = 0;
  m_length = 0;
}

void LineParser::rewindCursor(size_t number)
{
  char rewindCommand[12];
  rewindCommand[0] = '\x1B';
  rewindCommand[1] = '[';

  char *offset = TerminalHelpers::int2str(rewindCommand + 2, number);
  *offset++ = 'D';

  m_terminal.write(rewindCommand, offset - rewindCommand);
}
