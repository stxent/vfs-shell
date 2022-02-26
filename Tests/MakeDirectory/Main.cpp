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

class TestMakeDirectoryApplication: public TestApplication
{
public:
  TestMakeDirectoryApplication(Interface *client, Interface *host) :
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

class MakeDirectoryTest: public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(MakeDirectoryTest);
  CPPUNIT_TEST(testErrorAlreadyExists);
  CPPUNIT_TEST(testErrorEmptyCommand);
  CPPUNIT_TEST(testErrorIncorrectParent);
  CPPUNIT_TEST(testErrorNoParent);
  CPPUNIT_TEST(testErrorReadOnlyParent);
  CPPUNIT_TEST(testHelpMessage);
  CPPUNIT_TEST(testSimpleDirectoryCreation);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testErrorAlreadyExists();
  void testErrorEmptyCommand();
  void testErrorIncorrectParent();
  void testErrorNoParent();
  void testErrorReadOnlyParent();
  void testHelpMessage();
  void testSimpleDirectoryCreation();

private:
  uv_loop_t *m_loop{nullptr};
  Interrupt *m_listener{nullptr};
  Interface *m_appInterface{nullptr};
  Interface *m_testInterface{nullptr};
  TestApplication *m_application{nullptr};

  std::thread *m_appThread{nullptr};
  std::thread *m_loopThread{nullptr};
};

void MakeDirectoryTest::setUp()
{
  m_loop = uv_default_loop();
  CPPUNIT_ASSERT(m_loop != nullptr);

  m_listener = TestApplication::makeSignalListener(SIGUSR1, onSignalReceived, m_loop);
  CPPUNIT_ASSERT(m_listener != nullptr);
  m_appInterface = TestApplication::makeUdpInterface("127.0.0.1", 8000, 8001);
  CPPUNIT_ASSERT(m_appInterface != nullptr);
  m_testInterface = TestApplication::makeUdpInterface("127.0.0.1", 8001, 8000);
  CPPUNIT_ASSERT(m_testInterface != nullptr);

  m_application = new TestMakeDirectoryApplication(m_appInterface, m_testInterface);

  m_loopThread = new std::thread{TestApplication::runEventLoop, m_loop};
  m_appThread = new std::thread{TestApplication::runShell, m_application};

  m_application->waitShellResponse();
}

void MakeDirectoryTest::tearDown()
{
  m_application->sendShellCommand("exit");

  m_appThread->join();
  delete m_appThread;

  m_loopThread->join();
  delete m_loopThread;
}

void MakeDirectoryTest::testErrorAlreadyExists()
{
  m_application->sendShellCommand("mkdir foo");
  m_application->waitShellResponse();

  m_application->sendShellCommand("mkdir foo");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "node already exists");
  CPPUNIT_ASSERT(result == true);

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_EXIST));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void MakeDirectoryTest::testErrorEmptyCommand()
{
  m_application->sendShellCommand("mkdir");
  m_application->waitShellResponse();

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_VALUE));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void MakeDirectoryTest::testErrorIncorrectParent()
{
  m_application->sendShellCommand("mkdir /bin/exit/foo");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "directory creation failed");
  CPPUNIT_ASSERT(result == true);

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_INVALID));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void MakeDirectoryTest::testErrorReadOnlyParent()
{
  m_application->sendShellCommand("chmod -w /dev");
  m_application->waitShellResponse();

  m_application->sendShellCommand("mkdir /dev/foo");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "directory creation failed");
  CPPUNIT_ASSERT(result == true);

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_ACCESS));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void MakeDirectoryTest::testErrorNoParent()
{
  m_application->sendShellCommand("mkdir /undefined/foo");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "root node not found");
  CPPUNIT_ASSERT(result == true);

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_ENTRY));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void MakeDirectoryTest::testHelpMessage()
{
  m_application->sendShellCommand("mkdir --help");
  const auto response = m_application->waitShellResponse();

  const auto result = TestApplication::responseContainsText(response, "Usage");
  CPPUNIT_ASSERT(result == true);
}

void MakeDirectoryTest::testSimpleDirectoryCreation()
{
  m_application->sendShellCommand("mkdir foo");
  m_application->waitShellResponse();

  m_application->sendShellCommand("ls");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "foo");
  CPPUNIT_ASSERT(result == true);
}

CPPUNIT_TEST_SUITE_REGISTRATION(MakeDirectoryTest);

int main(int, char *[])
{
  CPPUNIT_NS::Test * const suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();

  CppUnit::TextUi::TestRunner runner;
  runner.addTest(suite);

  const bool sucessful = runner.run();
  return sucessful ? EXIT_SUCCESS : EXIT_FAILURE;
}
