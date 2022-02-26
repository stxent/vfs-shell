/*
 * Main.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "TestApplication.hpp"
#include "Shell/Scripts/ChangeModeScript.hpp"
#include "Shell/Scripts/GetEnvScript.hpp"
#include "Shell/Scripts/ListNodesScript.hpp"
#include "Shell/Scripts/MakeDirectoryScript.hpp"
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

class TestChangeModeApplication: public TestApplication
{
public:
  TestChangeModeApplication(Interface *client, Interface *host) :
    TestApplication{client, host}
  {
  }

  void bootstrap() override
  {
    TestApplication::bootstrap();

    m_initializer.attach<ChangeModeScript>();
    m_initializer.attach<GetEnvScript>();
    m_initializer.attach<ListNodesScript>();
    m_initializer.attach<MakeDirectoryScript>();
  }
};

class ChangeModeTest: public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(ChangeModeTest);
  CPPUNIT_TEST(testErrorIncorrectArguments);
  CPPUNIT_TEST(testErrorNoNode);
  CPPUNIT_TEST(testHelpMessage);
  CPPUNIT_TEST(testModeChange);
  CPPUNIT_TEST(testRecursiveModeChange);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testErrorIncorrectArguments();
  void testErrorNoNode();
  void testHelpMessage();
  void testModeChange();
  void testRecursiveModeChange();

private:
  uv_loop_t *m_loop{nullptr};
  Interrupt *m_listener{nullptr};
  Interface *m_appInterface{nullptr};
  Interface *m_testInterface{nullptr};
  TestApplication *m_application{nullptr};

  std::thread *m_appThread{nullptr};
  std::thread *m_loopThread{nullptr};
};

void ChangeModeTest::setUp()
{
  m_loop = uv_default_loop();
  CPPUNIT_ASSERT(m_loop != nullptr);

  m_listener = TestApplication::makeSignalListener(SIGUSR1, onSignalReceived, m_loop);
  CPPUNIT_ASSERT(m_listener != nullptr);
  m_appInterface = TestApplication::makeUdpInterface("127.0.0.1", 8000, 8001);
  CPPUNIT_ASSERT(m_appInterface != nullptr);
  m_testInterface = TestApplication::makeUdpInterface("127.0.0.1", 8001, 8000);
  CPPUNIT_ASSERT(m_testInterface != nullptr);

  m_application = new TestChangeModeApplication(m_appInterface, m_testInterface);

  m_loopThread = new std::thread{TestApplication::runEventLoop, m_loop};
  m_appThread = new std::thread{TestApplication::runShell, m_application};

  m_application->waitShellResponse();
}

void ChangeModeTest::tearDown()
{
  m_application->sendShellCommand("exit");

  m_appThread->join();
  delete m_appThread;

  m_loopThread->join();
  delete m_loopThread;
}

void ChangeModeTest::testErrorIncorrectArguments()
{
  m_application->sendShellCommand("chmod");
  m_application->waitShellResponse();

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_VALUE));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void ChangeModeTest::testErrorNoNode()
{
  m_application->sendShellCommand("chmod +rw /undefined");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "node not found");
  CPPUNIT_ASSERT(result == true);

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_ENTRY));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void ChangeModeTest::testHelpMessage()
{
  m_application->sendShellCommand("chmod --help");
  const auto response = m_application->waitShellResponse();

  const auto result = TestApplication::responseContainsText(response, "Usage");
  CPPUNIT_ASSERT(result == true);
}

void ChangeModeTest::testModeChange()
{
  std::vector<std::string> response;
  bool result;

  m_application->sendShellCommand("mkdir /test");
  m_application->waitShellResponse();
  m_application->sendShellCommand("mkdir /test/dir");
  m_application->waitShellResponse();

  // Clear R bit

  m_application->sendShellCommand("chmod -r /test/dir");
  m_application->waitShellResponse();

  m_application->sendShellCommand("ls -l /test");
  response = m_application->waitShellResponse();
  result = TestApplication::responseContainsText(response, "-w");
  CPPUNIT_ASSERT(result == true);

  // Set R bit, clear W bit

  m_application->sendShellCommand("chmod +r-w /test/dir");
  m_application->waitShellResponse();

  m_application->sendShellCommand("ls -l /test");
  response = m_application->waitShellResponse();
  result = TestApplication::responseContainsText(response, "r-");
  CPPUNIT_ASSERT(result == true);

  // Restore RW bits

  m_application->sendShellCommand("chmod +wr /test/dir");
  m_application->waitShellResponse();

  m_application->sendShellCommand("ls -l /test");
  response = m_application->waitShellResponse();
  result = TestApplication::responseContainsText(response, "rw");
  CPPUNIT_ASSERT(result == true);
}

void ChangeModeTest::testRecursiveModeChange()
{
  // TODO
  m_application->sendShellCommand("chmod -R +rw /dev");
  m_application->waitShellResponse();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ChangeModeTest);

int main(int, char *[])
{
  CPPUNIT_NS::Test * const suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();

  CppUnit::TextUi::TestRunner runner;
  runner.addTest(suite);

  const bool sucessful = runner.run();
  return sucessful ? EXIT_SUCCESS : EXIT_FAILURE;
}
