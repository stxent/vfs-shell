/*
 * Tests/Terminal/MockTerminal.hpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_TESTS_TERMINAL_MOCKTERMINAL_HPP_
#define VFS_SHELL_TESTS_TERMINAL_MOCKTERMINAL_HPP_

#include "Shell/Terminal.hpp"
#include <xcore/containers/byte_queue.h>
#include <string>

class Script;

class MockTerminal: public Terminal
{
public:
  MockTerminal(bool = false);
  virtual ~MockTerminal();

  virtual void subscribe(Script *) override;
  virtual void unsubscribe(const Script *) override;
  virtual size_t read(char *, size_t) override;
  virtual size_t write(const char *, size_t) override;

  std::string hostRead();
  void hostWrite(const std::string &);

private:
  static constexpr size_t BUFFER_SIZE{128};
  static constexpr size_t QUEUE_SIZE{1024};

  ByteQueue rxQueue;
  ByteQueue txQueue;
};

#endif // VFS_SHELL_TESTS_TERMINAL_MOCKTERMINAL_HPP_
