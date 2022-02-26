/*
 * Main.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "TestApplication.hpp"
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

class TestListNodesApplication: public TestApplication
{
public:
  TestListNodesApplication(Interface *client, Interface *host) :
    TestApplication{client, host}
  {
  }

  void bootstrap() override
  {
    TestApplication::bootstrap();

    m_initializer.attach<GetEnvScript>();
    m_initializer.attach<ListNodesScript>();
  }
};

class ListNodesTest: public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(ListNodesTest);
  CPPUNIT_TEST(testErrorNoDescendants);
  CPPUNIT_TEST(testErrorNoNode);
  CPPUNIT_TEST(testExtendedInfo);
  CPPUNIT_TEST(testHelpMessage);
  CPPUNIT_TEST(testHumanReadableSize);
  CPPUNIT_TEST(testListMultipleNodes);
  CPPUNIT_TEST(testNodeSize);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testErrorNoDescendants();
  void testErrorNoNode();
  void testExtendedInfo();
  void testHelpMessage();
  void testHumanReadableSize();
  void testListMultipleNodes();
  void testNodeSize();

private:
  uv_loop_t *m_loop{nullptr};
  Interrupt *m_listener{nullptr};
  Interface *m_appInterface{nullptr};
  Interface *m_testInterface{nullptr};
  TestApplication *m_application{nullptr};

  std::thread *m_appThread{nullptr};
  std::thread *m_loopThread{nullptr};
};

void ListNodesTest::setUp()
{
  m_loop = uv_default_loop();
  CPPUNIT_ASSERT(m_loop != nullptr);

  m_listener = TestApplication::makeSignalListener(SIGUSR1, onSignalReceived, m_loop);
  CPPUNIT_ASSERT(m_listener != nullptr);
  m_appInterface = TestApplication::makeUdpInterface("127.0.0.1", 8000, 8001);
  CPPUNIT_ASSERT(m_appInterface != nullptr);
  m_testInterface = TestApplication::makeUdpInterface("127.0.0.1", 8001, 8000);
  CPPUNIT_ASSERT(m_testInterface != nullptr);

  m_application = new TestListNodesApplication(m_appInterface, m_testInterface);

  m_application->makeDataNode("/test.bin", 1572864, 'A');

  m_loopThread = new std::thread{TestApplication::runEventLoop, m_loop};
  m_appThread = new std::thread{TestApplication::runShell, m_application};

  m_application->waitShellResponse();
}

void ListNodesTest::tearDown()
{
  m_application->sendShellCommand("exit");

  m_appThread->join();
  delete m_appThread;

  m_loopThread->join();
  delete m_loopThread;
}

void ListNodesTest::testErrorNoDescendants()
{
  m_application->sendShellCommand("ls /test.bin");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "node has no descendants");
  CPPUNIT_ASSERT(result == true);

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_OK));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void ListNodesTest::testErrorNoNode()
{
  m_application->sendShellCommand("ls /undefined");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "node not found");
  CPPUNIT_ASSERT(result == true);

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_ENTRY));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void ListNodesTest::testHumanReadableSize()
{
  m_application->sendShellCommand("ls -l -h");
  const auto response = m_application->waitShellResponse();

  const auto result0 = TestApplication::responseContainsText(response, "test.bin");
  CPPUNIT_ASSERT(result0 == true);
  const auto result1 = TestApplication::responseContainsText(response, "1.5M");
  CPPUNIT_ASSERT(result1 == true);
}

void ListNodesTest::testHelpMessage()
{
  m_application->sendShellCommand("ls --help");
  const auto response = m_application->waitShellResponse();

  const auto result = TestApplication::responseContainsText(response, "Usage");
  CPPUNIT_ASSERT(result == true);
}

void ListNodesTest::testExtendedInfo()
{
  m_application->sendShellCommand("ls -l -i bin");
  const auto response = m_application->waitShellResponse();

  const auto result = TestApplication::responseContainsText(response, "ls");
  CPPUNIT_ASSERT(result == true);
}

void ListNodesTest::testListMultipleNodes()
{
  m_application->sendShellCommand("ls bin dev");
  const auto response = m_application->waitShellResponse();

  const auto result0 = TestApplication::responseContainsText(response, "bin");
  CPPUNIT_ASSERT(result0 == true);
  const auto result1 = TestApplication::responseContainsText(response, "dev");
  CPPUNIT_ASSERT(result1 == true);
  const auto result2 = TestApplication::responseContainsText(response, "ls");
  CPPUNIT_ASSERT(result2 == true);
}

void ListNodesTest::testNodeSize()
{
  m_application->sendShellCommand("ls -l");
  const auto response = m_application->waitShellResponse();

  const auto result0 = TestApplication::responseContainsText(response, "test.bin");
  CPPUNIT_ASSERT(result0 == true);
  const auto result1 = TestApplication::responseContainsText(response, "1572864");
  CPPUNIT_ASSERT(result1 == true);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ListNodesTest);

int main(int, char *[])
{
  CPPUNIT_NS::Test * const suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();

  CppUnit::TextUi::TestRunner runner;
  runner.addTest(suite);

  const bool sucessful = runner.run();
  return sucessful ? EXIT_SUCCESS : EXIT_FAILURE;
}
