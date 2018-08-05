/*
 * EscapeSeqParser.cpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <cassert>
#include "Shell/EscapeSeqParser.hpp"

EscapeSeqParser::EscapeSeqParser() :
  m_length{0},
  m_state{0}
{
}

EscapeSeqParser::Event EscapeSeqParser::event() const
{
  assert(m_length >= 1);

  const char type = m_sequence[m_length - 1];

  // TODO Implement cursor movement by N
  switch (type)
  {
    case 'A':
      return UP;

    case 'B':
      return DOWN;

    case 'C':
      return RIGHT;

    case 'D':
      return LEFT;

    default:
      break;
  }

  if (type == '~' && m_length >= 2)
  {
    const char subtype = m_sequence[m_length - 2];

    switch (subtype)
    {
      case '3':
        return DEL;

      default:
        break;
    }
  }

  return UNDEFINED;
}

EscapeSeqParser::Status EscapeSeqParser::parse(char c)
{
  Status status = Status::DISCARDED;

  // Parse CSI only
  if (c == '\x1B' && m_state == 0)
  {
    m_length = 0;
    m_state = c;
    status = Status::CONSUMED;
  }
  else if (m_state == '\x1B' && c == '[')
  {
    m_state = c;
    status = Status::CONSUMED;
  }
  else if (m_state == '[')
  {
    if (c >= '\x20' && c <= '\x3F')
    {
      // Parameter bytes or intermediate bytes
      if (m_length < m_sequence.size() - 2)
      {
        m_sequence[m_length++] = c;
        status = Status::CONSUMED;
      }
      else
      {
        // Sequence is too long, reset parser
        m_state = 0;
      }
    }
    else if (c >= '\x40' && c <= '\x7E')
    {
      // Final byte
      m_sequence[m_length++] = c;
      m_sequence[m_length] = '\0';
      m_state = 0;
      status = Status::COMPLETED;
    }
    else
    {
      // Incorrect character, reset parser
      m_state = 0;
    }
  }
  else
  {
    // Reset parser
    m_state = 0;
  }

  return status;
}
