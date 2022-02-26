/*
 * Main.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "LimitedTestNode.hpp"
#include "SyncedTestNode.hpp"
#include "TestApplication.hpp"
#include "Shell/Scripts/DirectDataScript.hpp"
#include "Shell/Scripts/GetEnvScript.hpp"
#include "Shell/Scripts/ListNodesScript.hpp"
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

class TestDirectDataApplication: public TestApplication
{
public:
  TestDirectDataApplication(Interface *client, Interface *host) :
    TestApplication{client, host}
  {
  }

  void bootstrap() override
  {
    TestApplication::bootstrap();

    m_initializer.attach<DirectDataScript<BUFFER_SIZE>>();
    m_initializer.attach<GetEnvScript>();
    m_initializer.attach<ListNodesScript>();
  }
};

class DirectDataTest: public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(DirectDataTest);
  CPPUNIT_TEST(testCopyInterrupt);
  CPPUNIT_TEST(testCopyReadError);
  CPPUNIT_TEST(testErrorIncorrectArguments);
  CPPUNIT_TEST(testErrorIncorrectBlockSize);
  CPPUNIT_TEST(testErrorNoDestinationArgument);
  CPPUNIT_TEST(testErrorNoDestinationNode);
  CPPUNIT_TEST(testErrorNoSourceArgument);
  CPPUNIT_TEST(testErrorNoSourceNode);
  CPPUNIT_TEST(testHelpMessage);
  CPPUNIT_TEST(testMinimalNodeCopy);
  CPPUNIT_TEST(testPartialNodeCopy);
  CPPUNIT_TEST(testPositionalNodeCopy);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testCopyInterrupt();
  void testCopyReadError();
  void testErrorIncorrectArguments();
  void testErrorIncorrectBlockSize();
  void testErrorNoDestinationArgument();
  void testErrorNoDestinationNode();
  void testErrorNoSourceArgument();
  void testErrorNoSourceNode();
  void testHelpMessage();
  void testMinimalNodeCopy();
  void testPartialNodeCopy();
  void testPositionalNodeCopy();

private:
  uv_loop_t *m_loop{nullptr};
  Interrupt *m_listener{nullptr};
  Interface *m_appInterface{nullptr};
  Interface *m_testInterface{nullptr};
  TestApplication *m_application{nullptr};

  std::thread *m_appThread{nullptr};
  std::thread *m_loopThread{nullptr};

  LimitedTestNode *m_nodePartialTest{nullptr};
  SyncedTestNode *m_nodeInterruptTest{nullptr};
  SyncedTestNode *m_nodeReadTest{nullptr};
  unsigned int m_iteration{0};

  Result onInterruptTestCallback();
  Result onReadFailureTestCallback();
};

void DirectDataTest::setUp()
{
  bool ok;

  m_loop = uv_default_loop();
  CPPUNIT_ASSERT(m_loop != nullptr);

  m_listener = TestApplication::makeSignalListener(SIGUSR1, onSignalReceived, m_loop);
  CPPUNIT_ASSERT(m_listener != nullptr);
  m_appInterface = TestApplication::makeUdpInterface("127.0.0.1", 8000, 8001);
  CPPUNIT_ASSERT(m_appInterface != nullptr);
  m_testInterface = TestApplication::makeUdpInterface("127.0.0.1", 8001, 8000);
  CPPUNIT_ASSERT(m_testInterface != nullptr);

  m_application = new TestDirectDataApplication(m_appInterface, m_testInterface);

  m_nodeInterruptTest = new SyncedTestNode{[this](){ return onInterruptTestCallback(); }};
  ok = m_nodeInterruptTest->reserve(65536, 'z');
  CPPUNIT_ASSERT(ok == true);

  m_nodeReadTest = new SyncedTestNode{[this](){ return onReadFailureTestCallback(); }};
  ok = m_nodeReadTest->reserve(65536, 'a');
  CPPUNIT_ASSERT(ok == true);

  m_nodePartialTest = new LimitedTestNode{10000};
  m_iteration = 0;

  m_application->injectNode(m_nodePartialTest, "/partial_test.bin");
  m_application->injectNode(m_nodeInterruptTest, "/int_test.bin");
  m_application->injectNode(m_nodeReadTest, "/read_test.bin");
  m_application->makeDataNode("/empty.bin", 0, '\0');
  m_application->makeDataNode("/test.bin", 65536, 'A');

  m_loopThread = new std::thread{TestApplication::runEventLoop, m_loop};
  m_appThread = new std::thread{TestApplication::runShell, m_application};

  m_application->waitShellResponse();
}

void DirectDataTest::tearDown()
{
  m_application->sendShellCommand("exit");

  m_appThread->join();
  delete m_appThread;

  m_loopThread->join();
  delete m_loopThread;
}

void DirectDataTest::testCopyInterrupt()
{
  m_application->sendShellCommand("dd --if /int_test.bin --of /dev/test_2.bin --bs 1024");
  m_application->waitShellResponse();

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_TIMEOUT));
  CPPUNIT_ASSERT(returnValueFound == true);

  m_application->sendShellCommand("ls -l /dev");
  const auto response = m_application->waitShellResponse();

  const auto result0 = TestApplication::responseContainsText(response, "test_2.bin");
  CPPUNIT_ASSERT(result0 == true);
  const auto result1 = TestApplication::responseContainsText(response, "4096");
  CPPUNIT_ASSERT(result1 == true);
}

void DirectDataTest::testCopyReadError()
{
  m_application->sendShellCommand("dd --if /read_test.bin --of /dev/test_2.bin --bs 1024");
  m_application->waitShellResponse();

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_INTERFACE));
  CPPUNIT_ASSERT(returnValueFound == true);

  m_application->sendShellCommand("ls -l /dev");
  const auto response = m_application->waitShellResponse();

  const auto result0 = TestApplication::responseContainsText(response, "test_2.bin");
  CPPUNIT_ASSERT(result0 == true);
  const auto result1 = TestApplication::responseContainsText(response, "4096");
  CPPUNIT_ASSERT(result1 == true);
}

void DirectDataTest::testErrorIncorrectArguments()
{
  m_application->sendShellCommand("dd incorrect");
  m_application->waitShellResponse();

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_VALUE));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void DirectDataTest::testErrorIncorrectBlockSize()
{
  m_application->sendShellCommand("dd --if /test.bin --of /test_2.bin --bs 1073741824");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "block size is too big");
  CPPUNIT_ASSERT(result == true);

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_VALUE));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void DirectDataTest::testErrorNoDestinationArgument()
{
  m_application->sendShellCommand("dd --if /test.bin");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response,
      "missing destination file operand");
  CPPUNIT_ASSERT(result == true);

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_VALUE));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void DirectDataTest::testErrorNoDestinationNode()
{
  m_application->sendShellCommand("dd --if /test.bin --of /undefined/test_2.bin");
  const auto response = m_application->waitShellResponse();

  const auto result = TestApplication::responseContainsText(response, "open failed");
  CPPUNIT_ASSERT(result == true);
}

void DirectDataTest::testErrorNoSourceArgument()
{
  m_application->sendShellCommand("dd --of /test_2.bin");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response,
      "missing source file operand");
  CPPUNIT_ASSERT(result == true);

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_VALUE));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void DirectDataTest::testErrorNoSourceNode()
{
  m_application->sendShellCommand("dd --if /undefined --of /test_2.bin");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "open failed");
  CPPUNIT_ASSERT(result == true);

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_ENTRY));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void DirectDataTest::testHelpMessage()
{
  m_application->sendShellCommand("dd --help");
  const auto response = m_application->waitShellResponse();

  const auto result = TestApplication::responseContainsText(response, "Usage");
  CPPUNIT_ASSERT(result == true);
}

void DirectDataTest::testMinimalNodeCopy()
{
  m_application->sendShellCommand("dd --if /test.bin --of /dev/test_2.bin");
  m_application->waitShellResponse();

  m_application->sendShellCommand("ls -l /dev");
  const auto response = m_application->waitShellResponse();

  const auto result0 = TestApplication::responseContainsText(response, "test_2.bin");
  CPPUNIT_ASSERT(result0 == true);
  const auto result1 = TestApplication::responseContainsText(response, "65536");
  CPPUNIT_ASSERT(result1 == true);
}

void DirectDataTest::testPartialNodeCopy()
{
  m_application->sendShellCommand("dd --if /test.bin --of /partial_test.bin");
  m_application->waitShellResponse();

  m_application->sendShellCommand("ls -l");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "10000");
  CPPUNIT_ASSERT(result == true);
}

void DirectDataTest::testPositionalNodeCopy()
{
  // First pass - initial copy

  m_application->sendShellCommand("dd --if /test.bin --of /dev/test_2.bin --skip 32 --bs 1024 --count 16");
  m_application->waitShellResponse();

  m_application->sendShellCommand("ls -l /dev");
  const auto responseA = m_application->waitShellResponse();

  const auto resultA0 = TestApplication::responseContainsText(responseA, "test_2.bin");
  CPPUNIT_ASSERT(resultA0 == true);
  const auto resultA1 = TestApplication::responseContainsText(responseA, "16384");
  CPPUNIT_ASSERT(resultA1 == true);

  // Second pass - append data

  m_application->sendShellCommand("dd --if /test.bin --of /dev/test_2.bin --skip 32 --bs 1024 --count 16 --seek 16");
  m_application->waitShellResponse();

  m_application->sendShellCommand("ls -l /dev");
  const auto responseB = m_application->waitShellResponse();

  const auto resultB0 = TestApplication::responseContainsText(responseB, "test_2.bin");
  CPPUNIT_ASSERT(resultB0 == true);
  const auto resultB1 = TestApplication::responseContainsText(responseB, "32768");
  CPPUNIT_ASSERT(resultB1 == true);
}

Result DirectDataTest::onInterruptTestCallback()
{
  if (++m_iteration == 4)
    m_application->sendShellBuffer("\x03", 1);
  else
    m_application->sendShellBuffer(" ", 1);

  usleep(10000);
  m_nodeInterruptTest->post();

  return E_OK;
}

Result DirectDataTest::onReadFailureTestCallback()
{
  m_nodeReadTest->post();

  if (++m_iteration == 5)
    return E_INTERFACE;
  else
    return E_OK;
}

CPPUNIT_TEST_SUITE_REGISTRATION(DirectDataTest);

int main(int, char *[])
{
  CPPUNIT_NS::Test * const suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();

  CppUnit::TextUi::TestRunner runner;
  runner.addTest(suite);

  const bool sucessful = runner.run();
  return sucessful ? EXIT_SUCCESS : EXIT_FAILURE;
}
