/*
 * Main.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "TestApplication.hpp"
#include "Shell/Scripts/ChangeModeScript.hpp"
#include "Shell/Scripts/GetEnvScript.hpp"
#include "Shell/Scripts/ListNodesScript.hpp"
#include "Shell/Scripts/RemoveNodesScript.hpp"
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

class TestRemoveNodesApplication: public TestApplication
{
public:
  TestRemoveNodesApplication(Interface *client, Interface *host) :
    TestApplication{client, host}
  {
  }

  void bootstrap() override
  {
    TestApplication::bootstrap();

    m_initializer.attach<ChangeModeScript>();
    m_initializer.attach<GetEnvScript>();
    m_initializer.attach<ListNodesScript>();
    m_initializer.attach<RemoveNodesScript>();
  }
};

class RemoveNodesTest: public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(RemoveNodesTest);
  CPPUNIT_TEST(testDataNodeRemove);
  CPPUNIT_TEST(testDirectoryRemove);
  CPPUNIT_TEST(testErrorDirectoryNode);
  CPPUNIT_TEST(testErrorNoNode);
  CPPUNIT_TEST(testErrorReadOnlyNode);
  CPPUNIT_TEST(testErrorReadOnlyParent);
  CPPUNIT_TEST(testHelpMessage);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testDataNodeRemove();
  void testDirectoryRemove();
  void testErrorDirectoryNode();
  void testErrorNoNode();
  void testErrorReadOnlyNode();
  void testErrorReadOnlyParent();
  void testHelpMessage();

private:
  uv_loop_t *m_loop{nullptr};
  Interrupt *m_listener{nullptr};
  Interface *m_appInterface{nullptr};
  Interface *m_testInterface{nullptr};
  TestApplication *m_application{nullptr};

  std::thread *m_appThread{nullptr};
  std::thread *m_loopThread{nullptr};
};

void RemoveNodesTest::setUp()
{
  m_loop = uv_default_loop();
  CPPUNIT_ASSERT(m_loop != nullptr);

  m_listener = TestApplication::makeSignalListener(SIGUSR1, onSignalReceived, m_loop);
  CPPUNIT_ASSERT(m_listener != nullptr);
  m_appInterface = TestApplication::makeUdpInterface("127.0.0.1", 8000, 8001);
  CPPUNIT_ASSERT(m_appInterface != nullptr);
  m_testInterface = TestApplication::makeUdpInterface("127.0.0.1", 8001, 8000);
  CPPUNIT_ASSERT(m_testInterface != nullptr);

  m_application = new TestRemoveNodesApplication(m_appInterface, m_testInterface);

  m_application->makeDataNode("/test.bin", 65536, 'A');

  m_loopThread = new std::thread{TestApplication::runEventLoop, m_loop};
  m_appThread = new std::thread{TestApplication::runShell, m_application};

  m_application->waitShellResponse();
}

void RemoveNodesTest::tearDown()
{
  m_application->sendShellCommand("exit");

  m_appThread->join();
  delete m_appThread;

  m_loopThread->join();
  delete m_loopThread;
}

void RemoveNodesTest::testDataNodeRemove()
{
  m_application->sendShellCommand("rm /test.bin");
  m_application->waitShellResponse();

  m_application->sendShellCommand("ls");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "test.bin");
  CPPUNIT_ASSERT(result == false);
}

void RemoveNodesTest::testDirectoryRemove()
{
  m_application->sendShellCommand("rm -r /dev");
  m_application->waitShellResponse();

  m_application->sendShellCommand("ls");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "dev");
  CPPUNIT_ASSERT(result == false);
}

void RemoveNodesTest::testErrorDirectoryNode()
{
  m_application->sendShellCommand("rm /dev");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "directory node ignored");
  CPPUNIT_ASSERT(result == true);

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_ENTRY));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void RemoveNodesTest::testErrorNoNode()
{
  m_application->sendShellCommand("rm /undefined /test");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "node not found");
  CPPUNIT_ASSERT(result == true);

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_ENTRY));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void RemoveNodesTest::testErrorReadOnlyNode()
{
  // Try to remove read-only node

  m_application->sendShellCommand("chmod -w /test.bin");
  m_application->waitShellResponse();

  m_application->sendShellCommand("rm /test.bin");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "deletion failed");
  CPPUNIT_ASSERT(result == true);

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_ACCESS));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void RemoveNodesTest::testErrorReadOnlyParent()
{
  m_application->makeDataNode("/dev/test_2.bin", 65536, 'z');

  // Try to remove from read-only directory

  m_application->sendShellCommand("chmod -w /dev");
  m_application->waitShellResponse();

  m_application->sendShellCommand("rm /dev/test_2.bin");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "deletion failed");
  CPPUNIT_ASSERT(result == true);

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_ACCESS));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void RemoveNodesTest::testHelpMessage()
{
  m_application->sendShellCommand("rm --help");
  const auto response = m_application->waitShellResponse();

  const auto result = TestApplication::responseContainsText(response, "Usage");
  CPPUNIT_ASSERT(result == true);
}

CPPUNIT_TEST_SUITE_REGISTRATION(RemoveNodesTest);

int main(int, char *[])
{
  CPPUNIT_NS::Test * const suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();

  CppUnit::TextUi::TestRunner runner;
  runner.addTest(suite);

  const bool sucessful = runner.run();
  return sucessful ? EXIT_SUCCESS : EXIT_FAILURE;
}
