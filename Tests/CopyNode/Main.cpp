/*
 * Main.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "LimitedTestNode.hpp"
#include "TestApplication.hpp"
#include "Shell/Scripts/CopyNodeScript.hpp"
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

class TestCopyNodeApplication: public TestApplication
{
public:
  TestCopyNodeApplication(Interface *client, Interface *host) :
    TestApplication{client, host}
  {
  }

  void bootstrap() override
  {
    TestApplication::bootstrap();

    m_initializer.attach<CopyNodeScript<BUFFER_SIZE>>();
    m_initializer.attach<GetEnvScript>();
    m_initializer.attach<ListNodesScript>();
  }
};

class CopyNodeTest: public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(CopyNodeTest);
  CPPUNIT_TEST(testErrorNoDestinationArgument);
  CPPUNIT_TEST(testErrorNoDestinationNode);
  CPPUNIT_TEST(testErrorNoSourceNode);
  CPPUNIT_TEST(testHelpMessage);
  CPPUNIT_TEST(testPartialCopy);
  CPPUNIT_TEST(testSimpleCopy);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testErrorNoDestinationArgument();
  void testErrorNoDestinationNode();
  void testErrorNoSourceNode();
  void testHelpMessage();
  void testPartialCopy();
  void testSimpleCopy();

private:
  uv_loop_t *m_loop{nullptr};
  Interrupt *m_listener{nullptr};
  Interface *m_appInterface{nullptr};
  Interface *m_testInterface{nullptr};
  TestApplication *m_application{nullptr};

  std::thread *m_appThread{nullptr};
  std::thread *m_loopThread{nullptr};

  LimitedTestNode *m_nodeWriteTest{nullptr};
};

void CopyNodeTest::setUp()
{
  m_loop = uv_default_loop();
  CPPUNIT_ASSERT(m_loop != nullptr);

  m_listener = TestApplication::makeSignalListener(SIGUSR1, onSignalReceived, m_loop);
  CPPUNIT_ASSERT(m_listener != nullptr);
  m_appInterface = TestApplication::makeUdpInterface("127.0.0.1", 8000, 8001);
  CPPUNIT_ASSERT(m_appInterface != nullptr);
  m_testInterface = TestApplication::makeUdpInterface("127.0.0.1", 8001, 8000);
  CPPUNIT_ASSERT(m_testInterface != nullptr);

  m_application = new TestCopyNodeApplication(m_appInterface, m_testInterface);
  m_nodeWriteTest = new LimitedTestNode{10000};

  m_application->injectNode(m_nodeWriteTest, "/write_test.bin");
  m_application->makeDataNode("/test.bin", 65536, 'A');

  m_loopThread = new std::thread{TestApplication::runEventLoop, m_loop};
  m_appThread = new std::thread{TestApplication::runShell, m_application};

  m_application->waitShellResponse();
}

void CopyNodeTest::tearDown()
{
  m_application->sendShellCommand("exit");

  m_appThread->join();
  delete m_appThread;

  m_loopThread->join();
  delete m_loopThread;
}

void CopyNodeTest::testErrorNoDestinationArgument()
{
  m_application->sendShellCommand("cp test.bin");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response,
      "missing destination file operand");
  CPPUNIT_ASSERT(result == true);

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_VALUE));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void CopyNodeTest::testErrorNoDestinationNode()
{
  m_application->sendShellCommand("cp /test.bin /undefined/test_2.bin");
  const auto response = m_application->waitShellResponse();

  const auto result = TestApplication::responseContainsText(response, "open failed");
  CPPUNIT_ASSERT(result == true);
}

void CopyNodeTest::testErrorNoSourceNode()
{
  m_application->sendShellCommand("cp /undefined /test_2.bin");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "open failed");
  CPPUNIT_ASSERT(result == true);

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_ENTRY));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void CopyNodeTest::testHelpMessage()
{
  m_application->sendShellCommand("cp --help");
  const auto response = m_application->waitShellResponse();

  const auto result = TestApplication::responseContainsText(response, "Usage");
  CPPUNIT_ASSERT(result == true);
}

void CopyNodeTest::testPartialCopy()
{
  m_application->sendShellCommand("cp /test.bin /write_test.bin");
  m_application->waitShellResponse();

  m_application->sendShellCommand("ls -l");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "10000");
  CPPUNIT_ASSERT(result == true);
}

void CopyNodeTest::testSimpleCopy()
{
  m_application->sendShellCommand("cp /test.bin /test_2.bin");
  m_application->waitShellResponse();

  m_application->sendShellCommand("ls -l");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "test_2.bin");
  CPPUNIT_ASSERT(result == true);
}

CPPUNIT_TEST_SUITE_REGISTRATION(CopyNodeTest);

int main(int, char *[])
{
  CPPUNIT_NS::Test * const suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();

  CppUnit::TextUi::TestRunner runner;
  runner.addTest(suite);

  const bool sucessful = runner.run();
  return sucessful ? EXIT_SUCCESS : EXIT_FAILURE;
}
