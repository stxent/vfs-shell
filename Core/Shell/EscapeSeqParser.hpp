/*
 * Core/Shell/EscapeSeqParser.hpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_ESCAPESEQPARSER_HPP_
#define VFS_SHELL_CORE_SHELL_ESCAPESEQPARSER_HPP_

#include <array>

class EscapeSeqParser
{
public:
  enum Event
  {
    UNDEFINED,
    UP,
    DOWN,
    RIGHT,
    LEFT,
    DEL
  };

  enum class Status
  {
    COMPLETED,
    CONSUMED,
    DISCARDED
  };

  EscapeSeqParser();

  Event event() const;
  Status parse(char);

private:
  static constexpr size_t MAX_SEQUENCE_LENGTH{16};

  std::array<char, MAX_SEQUENCE_LENGTH> m_sequence;
  size_t m_length;
  char m_state;
};

#endif // VFS_SHELL_CORE_SHELL_ESCAPESEQPARSER_HPP_
