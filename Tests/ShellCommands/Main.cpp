/*
 * Main.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "TestApplication.hpp"
#include "Shell/Scripts/EchoScript.hpp"
#include "Shell/Scripts/GetEnvScript.hpp"
#include "Shell/Scripts/PrintRawDataScript.hpp"
#include "Shell/Scripts/SetEnvScript.hpp"
#include "Shell/Scripts/Shell.hpp"
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <uv.h>
#include <thread>

static void onUvWalk(uv_handle_t *handle, void *)
{
  deinit(uv_handle_get_data(handle));
}

static void onSignalReceived(void *argument)
{
  uv_walk(static_cast<uv_loop_t *>(argument), onUvWalk, 0);
}

class TestShellCommandsApplication: public TestApplication
{
public:
  TestShellCommandsApplication(Interface *client, Interface *host) :
    TestApplication{client, host, true}
  {
  }

  void bootstrap() override
  {
    TestApplication::bootstrap();

    m_initializer.attach<EchoScript>();
    m_initializer.attach<GetEnvScript>();
    m_initializer.attach<PrintRawDataScript<BUFFER_SIZE>>();
    m_initializer.attach<SetEnvScript>();
  }
};

class ShellCommandsTest: public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(ShellCommandsTest);
  CPPUNIT_TEST(testEmptyCommand);
  CPPUNIT_TEST(testErrorNoFileScript);
  CPPUNIT_TEST(testErrorNoScript);
  CPPUNIT_TEST(testFileScript);
  CPPUNIT_TEST(testFileScriptOutput);
  CPPUNIT_TEST(testFileScriptOutputAppend);
  CPPUNIT_TEST(testFileScriptOverwrite);
  CPPUNIT_TEST(testInnerShell);
  CPPUNIT_TEST(testScriptWithRelativePath);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testEmptyCommand();
  void testErrorNoFileScript();
  void testErrorNoScript();
  void testFileScript();
  void testFileScriptOutput();
  void testFileScriptOutputAppend();
  void testFileScriptOverwrite();
  void testInnerShell();
  void testScriptWithRelativePath();

private:
  uv_loop_t *m_loop{nullptr};
  Interrupt *m_listener{nullptr};
  Interface *m_appInterface{nullptr};
  Interface *m_testInterface{nullptr};
  TestApplication *m_application{nullptr};

  std::thread *m_appThread{nullptr};
  std::thread *m_loopThread{nullptr};
};

void ShellCommandsTest::setUp()
{
  m_loop = uv_default_loop();
  CPPUNIT_ASSERT(m_loop != nullptr);

  m_listener = TestApplication::makeSignalListener(SIGUSR1, onSignalReceived, m_loop);
  CPPUNIT_ASSERT(m_listener != nullptr);
  m_appInterface = TestApplication::makeUdpInterface("127.0.0.1", 8000, 8001);
  CPPUNIT_ASSERT(m_appInterface != nullptr);
  m_testInterface = TestApplication::makeUdpInterface("127.0.0.1", 8001, 8000);
  CPPUNIT_ASSERT(m_testInterface != nullptr);

  m_application = new TestShellCommandsApplication(m_appInterface, m_testInterface);

  m_application->makeDataNode("/script.sh", "echo first");
  m_application->makeDataNode("/script_2.sh", "echo second");
  m_application->makeDataNode("/script_3.sh", "/bin/echo third");

  m_loopThread = new std::thread{TestApplication::runEventLoop, m_loop};
  m_appThread = new std::thread{TestApplication::runShell, m_application};

  m_application->waitShellResponse();
}

void ShellCommandsTest::tearDown()
{
  m_application->sendShellCommand("exit");

  m_appThread->join();
  delete m_appThread;

  m_loopThread->join();
  delete m_loopThread;
}

void ShellCommandsTest::testEmptyCommand()
{
  m_application->sendShellCommand("");
  m_application->waitShellResponse();

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_OK));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void ShellCommandsTest::testErrorNoFileScript()
{
  m_application->sendShellCommand("sh /undefined.sh");
  m_application->waitShellResponse();

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_ENTRY));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void ShellCommandsTest::testErrorNoScript()
{
  m_application->sendShellCommand("undefined");
  m_application->waitShellResponse();

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_ENTRY));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void ShellCommandsTest::testFileScript()
{
  // Run a simple script
  m_application->sendShellCommand("sh /script.sh");
  const auto response = m_application->waitShellResponse();

  const auto result = TestApplication::responseContainsText(response, "first");
  CPPUNIT_ASSERT(result == true);
}

void ShellCommandsTest::testFileScriptOutput()
{
  // TODO valgrind --tool=memcheck --leak-check=yes --show-reachable=yes

  // Run a simple script
  m_application->sendShellCommand("sh /script.sh > /output.txt");
  m_application->waitShellResponse();

  // Check file content
  m_application->sendShellCommand("cat /output.txt");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "first");
  CPPUNIT_ASSERT(result == true);
}

void ShellCommandsTest::testFileScriptOutputAppend()
{
  // TODO valgrind --tool=memcheck --leak-check=yes --show-reachable=yes

  // Run a simple script
  m_application->sendShellCommand("sh /script.sh > /output.txt");
  m_application->waitShellResponse();

  // Append text
  m_application->sendShellCommand("sh /script_2.sh >> /output.txt");
  m_application->waitShellResponse();

  // Check file content
  m_application->sendShellCommand("cat /output.txt");
  const auto response = m_application->waitShellResponse();

  const auto result0 = TestApplication::responseContainsText(response, "first");
  CPPUNIT_ASSERT(result0 == true);
  const auto result1 = TestApplication::responseContainsText(response, "second");
  CPPUNIT_ASSERT(result1 == true);
}

void ShellCommandsTest::testFileScriptOverwrite()
{
  // TODO valgrind --tool=memcheck --leak-check=yes --show-reachable=yes

  // Run a simple script
  m_application->sendShellCommand("sh /script.sh > /output.txt");
  m_application->waitShellResponse();

  // Overwrite output file
  m_application->sendShellCommand("sh /script_2.sh > /output.txt");
  m_application->waitShellResponse();

  // Check file content
  m_application->sendShellCommand("cat /output.txt");
  const auto response = m_application->waitShellResponse();

  const auto result0 = TestApplication::responseContainsText(response, "first");
  CPPUNIT_ASSERT(result0 == false);
  const auto result1 = TestApplication::responseContainsText(response, "second");
  CPPUNIT_ASSERT(result1 == true);
}

void ShellCommandsTest::testInnerShell()
{
  // Start inner shell
  m_application->sendShellCommand("sh");
  m_application->waitShellResponse();

  // Run simple command
  m_application->sendShellCommand("echo test");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "test");
  CPPUNIT_ASSERT(result == true);

  // Exit inner shell
  m_application->sendShellCommand("exit");
  m_application->waitShellResponse();
}

void ShellCommandsTest::testScriptWithRelativePath()
{
  // Change PATH variable
  m_application->sendShellCommand("/bin/setenv PATH /dev");
  m_application->waitShellResponse();

  m_application->sendShellCommand("bin/sh /script_3.sh");
  const auto response = m_application->waitShellResponse();

  // Restore PATH variable
  m_application->sendShellCommand("/bin/setenv PATH /bin");
  m_application->waitShellResponse();

  const auto result = TestApplication::responseContainsText(response, "third");
  CPPUNIT_ASSERT(result == true);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ShellCommandsTest);

int main(int, char *[])
{
  CPPUNIT_NS::Test * const suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();

  CppUnit::TextUi::TestRunner runner;
  runner.addTest(suite);

  const bool sucessful = runner.run();
  return sucessful ? EXIT_SUCCESS : EXIT_FAILURE;
}
