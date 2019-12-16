/*
 * Core/Shell/LineParser.hpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_LINEPARSER_HPP_
#define VFS_SHELL_CORE_SHELL_LINEPARSER_HPP_

#include <array>

class Terminal;

class LineParser
{
public:
  enum class Status
  {
    COMPLETED,
    CONSUMED,
    DISCARDED,
    TERMINATED
  };

  LineParser(Terminal &, bool = true);

  bool eraseNext();
  bool erasePrevious();
  bool insert(char);
  void moveCursorLeft();
  void moveCursorRight();
  Status parse(char);
  void reset();

  char *data()
  {
    return m_command.data();
  }

  void disableEcho()
  {
    m_echo = false;
  }

  void enableEcho()
  {
    m_echo = true;
  }

  size_t length() const
  {
    return m_length;
  }

private:
  static constexpr size_t MAX_LINE_LENGTH{256};

  Terminal &m_terminal;
  std::array<char, MAX_LINE_LENGTH> m_command;
  size_t m_cursor;
  size_t m_length;
  char m_carriage;
  bool m_echo;

  void rewindCursor(size_t);
};

#endif // VFS_SHELL_CORE_SHELL_LINEPARSER_HPP_
