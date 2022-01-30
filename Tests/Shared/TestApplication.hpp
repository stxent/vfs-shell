/*
 * Tests/Shared/TestApplication.hpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_TESTS_SHARED_TESTAPPLICATION_HPP_
#define VFS_SHELL_TESTS_SHARED_TESTAPPLICATION_HPP_

#include "Shell/Initializer.hpp"
#include "Shell/SerialTerminal.hpp"
#include <halm/interrupt.h>
#include <xcore/interface.h>
#include <chrono>
#include <memory>
#include <string>
#include <vector>

class TestApplication
{
public:
  static constexpr size_t BUFFER_SIZE{4096};

  TestApplication(Interface *, Interface *);
  virtual ~TestApplication() = default;

  static void runEventLoop(void *);
  static void runShell(void *);

  void makeDataNode(const char *, size_t, char);
  void sendShellCommand(const char *);
  std::vector<std::string> waitShellResponse(std::chrono::milliseconds = std::chrono::milliseconds{1000});

  static Interrupt *makeSignalListener(int, void (*)(void *), void *);
  static Interface *makeUdpInterface(const char *, uint16_t, uint16_t);
  static bool responseContainsText(const std::vector<std::string> &, const std::string &);

protected:
  class SignalCaller
  {
  public:
    SignalCaller() = default;
    SignalCaller(const SignalCaller &) = delete;
    SignalCaller &operator=(const SignalCaller *) = delete;

    ~SignalCaller();
  };

  virtual void bootstrap();
  virtual void run();

  SignalCaller m_caller;

  Interface * const m_client;
  Interface * const m_host;

  std::unique_ptr<FsHandle, std::function<void (FsHandle *)>> m_filesystem;
  SerialTerminal m_terminal;
  Initializer m_initializer;
};

#endif // VFS_SHELL_TESTS_SHARED_TESTAPPLICATION_HPP_
