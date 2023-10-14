/*
 * Main.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "TestApplication.hpp"
#include "Shell/Scripts/EchoScript.hpp"
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

class TestLineParserApplication: public TestApplication
{
public:
  TestLineParserApplication(Interface *client, Interface *host) :
    TestApplication{client, host, true}
  {
  }

  void bootstrap() override
  {
    TestApplication::bootstrap();

    m_initializer.attach<EchoScript>();
  }
};

class LineParserTest: public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(LineParserTest);
  CPPUNIT_TEST(testBackspaceKey);
  CPPUNIT_TEST(testBackspaceKeyDiscard);
  CPPUNIT_TEST(testCursorMovement);
  CPPUNIT_TEST(testDeleteKey);
  CPPUNIT_TEST(testDeleteKeyDiscard);
  CPPUNIT_TEST(testIncorrectSequences);
  CPPUNIT_TEST(testInsertOverflow);
  CPPUNIT_TEST(testSystemSequences);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testBackspaceKey();
  void testBackspaceKeyDiscard();
  void testCursorMovement();
  void testDeleteKey();
  void testDeleteKeyDiscard();
  void testIncorrectSequences();
  void testInsertOverflow();
  void testSystemSequences();

private:
  uv_loop_t *m_loop{nullptr};
  Interrupt *m_listener{nullptr};
  Interface *m_appInterface{nullptr};
  Interface *m_testInterface{nullptr};
  TestApplication *m_application{nullptr};

  std::thread *m_appThread{nullptr};
  std::thread *m_loopThread{nullptr};
};

void LineParserTest::setUp()
{
  m_loop = uv_default_loop();
  CPPUNIT_ASSERT(m_loop != nullptr);

  m_listener = TestApplication::makeSignalListener(SIGUSR1, onSignalReceived, m_loop);
  CPPUNIT_ASSERT(m_listener != nullptr);
  m_appInterface = TestApplication::makeUdpInterface("127.0.0.1", 8000, 8001);
  CPPUNIT_ASSERT(m_appInterface != nullptr);
  m_testInterface = TestApplication::makeUdpInterface("127.0.0.1", 8001, 8000);
  CPPUNIT_ASSERT(m_testInterface != nullptr);

  m_application = new TestLineParserApplication(m_appInterface, m_testInterface);

  m_loopThread = new std::thread{TestApplication::runEventLoop, m_loop};
  m_appThread = new std::thread{TestApplication::runShell, m_application};

  m_application->waitShellResponse();
}

void LineParserTest::tearDown()
{
  m_application->sendShellBuffer("\x03", 1);

  m_appThread->join();
  delete m_appThread;

  m_loopThread->join();
  delete m_loopThread;
}

void LineParserTest::testBackspaceKey()
{
  m_application->sendShellText("echo none");

  // Erase "none"
  m_application->sendShellText("\b\b");
  m_application->sendShellText("\x7F\x7F");

  // Insert "test"
  m_application->sendShellText("test");

  // Run command
  m_application->sendShellText("\r\n");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "test");
  CPPUNIT_ASSERT(result == true);
}

void LineParserTest::testBackspaceKeyDiscard()
{
  m_application->sendShellText("none test");

  // Move left
  for (unsigned int i = 0; i < 5; ++i)
    m_application->sendShellBuffer("\x1B[D", 3);

  // Erase "none"
  m_application->sendShellText("\b\b\b\b");

  // Erase discard
  m_application->sendShellText("\b");

  // Insert "echo"
  m_application->sendShellText("echo");

  // Run command
  m_application->sendShellText("\r\n");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "test");
  CPPUNIT_ASSERT(result == true);
}

void LineParserTest::testCursorMovement()
{
  m_application->sendShellText("echo text argument");

  // Move left
  for (unsigned int i = 0; i < 13; ++i)
    m_application->sendShellBuffer("\x1B[D", 3);

  // Move right
  for (unsigned int i = 0; i < 4; ++i)
    m_application->sendShellText("\x1B[C");

  // Erase "text"
  m_application->sendShellText("\b\b\b\b");

  // Insert "foo"
  m_application->sendShellText("foo");

  // Run command
  m_application->sendShellText("\r\n");
  const auto response = m_application->waitShellResponse();

  const auto result0 = TestApplication::responseContainsText(response, "foo");
  CPPUNIT_ASSERT(result0 == true);
  const auto result1 = TestApplication::responseContainsText(response, "argument");
  CPPUNIT_ASSERT(result1 == true);
}

void LineParserTest::testDeleteKey()
{
  m_application->sendShellText("echo text argument");

  // Move left
  for (unsigned int i = 0; i < 13; ++i)
    m_application->sendShellText("\x1B[D");

  // Erase "text"
  for (unsigned int i = 0; i < 4; ++i)
    m_application->sendShellText("\x1B[3~");

  // Insert "foo"
  m_application->sendShellText("foo");

  // Run command
  m_application->sendShellText("\r\n");
  const auto response = m_application->waitShellResponse();

  const auto result0 = TestApplication::responseContainsText(response, "foo");
  CPPUNIT_ASSERT(result0 == true);
  const auto result1 = TestApplication::responseContainsText(response, "argument");
  CPPUNIT_ASSERT(result1 == true);
}

void LineParserTest::testDeleteKeyDiscard()
{
  m_application->sendShellText("echo none");

  // Move left
  for (unsigned int i = 0; i < 4; ++i)
    m_application->sendShellText("\x1B[D");

  // Erase "none"
  for (unsigned int i = 0; i < 4; ++i)
    m_application->sendShellText("\x1B[3~");

  // Erase discard
  m_application->sendShellText("\x1B[3~");

  // Insert "foo"
  m_application->sendShellText("test");

  // Run command
  m_application->sendShellText("\r\n");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "test");
  CPPUNIT_ASSERT(result == true);
}

void LineParserTest::testIncorrectSequences()
{
  std::vector<std::string> response;
  bool result;

  // Send incorrect sequence
  m_application->sendShellText("\x1B[\t~");
  response = m_application->waitShellResponse(2);
  result = TestApplication::responseContainsText(response, "\t~");
  CPPUNIT_ASSERT(result == true);

  // Send long sequence
  m_application->sendShellText("\x1B[000000000000001~");
  response = m_application->waitShellResponse(2);
  result = TestApplication::responseContainsText(response, "1~");
  CPPUNIT_ASSERT(result == true);
}

void LineParserTest::testInsertOverflow()
{
  static constexpr size_t MAX_LINE_LENGTH{256};

  m_application->sendShellText("echo test ");

  for (size_t i = 0; i <= MAX_LINE_LENGTH - 10; ++i)
    m_application->sendShellText("a");

  // Run command
  m_application->sendShellText("\r\n");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "test");
  CPPUNIT_ASSERT(result == true);
}

void LineParserTest::testSystemSequences()
{
  // Send F0
  m_application->sendShellText("\x1B[10~");

  // Send Up
  m_application->sendShellText("\x1B[A~");

  // Send Down
  m_application->sendShellText("\x1B[B~");
}

CPPUNIT_TEST_SUITE_REGISTRATION(LineParserTest);

int main(int, char *[])
{
  CPPUNIT_NS::Test * const suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();

  CppUnit::TextUi::TestRunner runner;
  runner.addTest(suite);

  const bool sucessful = runner.run();
  return sucessful ? EXIT_SUCCESS : EXIT_FAILURE;
}
