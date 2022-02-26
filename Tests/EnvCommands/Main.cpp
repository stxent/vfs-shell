/*
 * Main.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "TestApplication.hpp"
#include "Shell/Scripts/GetEnvScript.hpp"
#include "Shell/Scripts/ListEnvScript.hpp"
#include "Shell/Scripts/SetEnvScript.hpp"
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

class TestEnvCommandsApplication: public TestApplication
{
public:
  TestEnvCommandsApplication(Interface *client, Interface *host) :
    TestApplication{client, host}
  {
  }

  void bootstrap() override
  {
    TestApplication::bootstrap();

    m_initializer.attach<GetEnvScript>();
    m_initializer.attach<ListEnvScript>();
    m_initializer.attach<SetEnvScript>();
  }
};

class EnvCommandsTest: public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(EnvCommandsTest);
  CPPUNIT_TEST(testEnvListing);
  CPPUNIT_TEST(testEnvReadWrite);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testEnvListing();
  void testEnvReadWrite();

private:
  uv_loop_t *m_loop{nullptr};
  Interrupt *m_listener{nullptr};
  Interface *m_appInterface{nullptr};
  Interface *m_testInterface{nullptr};
  TestApplication *m_application{nullptr};

  std::thread *m_appThread{nullptr};
  std::thread *m_loopThread{nullptr};
};

void EnvCommandsTest::setUp()
{
  m_loop = uv_default_loop();
  CPPUNIT_ASSERT(m_loop != nullptr);

  m_listener = TestApplication::makeSignalListener(SIGUSR1, onSignalReceived, m_loop);
  CPPUNIT_ASSERT(m_listener != nullptr);
  m_appInterface = TestApplication::makeUdpInterface("127.0.0.1", 8000, 8001);
  CPPUNIT_ASSERT(m_appInterface != nullptr);
  m_testInterface = TestApplication::makeUdpInterface("127.0.0.1", 8001, 8000);
  CPPUNIT_ASSERT(m_testInterface != nullptr);

  m_application = new TestEnvCommandsApplication(m_appInterface, m_testInterface);

  m_loopThread = new std::thread{TestApplication::runEventLoop, m_loop};
  m_appThread = new std::thread{TestApplication::runShell, m_application};

  m_application->waitShellResponse();
}

void EnvCommandsTest::tearDown()
{
  m_application->sendShellCommand("exit");

  m_appThread->join();
  delete m_appThread;

  m_loopThread->join();
  delete m_loopThread;
}

void EnvCommandsTest::testEnvListing()
{
  m_application->sendShellCommand("env");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "PATH");
  CPPUNIT_ASSERT(result == true);
}

void EnvCommandsTest::testEnvReadWrite()
{
  std::vector<std::string> response;
  bool result;

  // Dynamically created variable

  m_application->sendShellCommand("setenv TEST pattern");
  m_application->waitShellResponse();

  m_application->sendShellCommand("getenv TEST");
  response = m_application->waitShellResponse();
  result = TestApplication::responseContainsText(response, "pattern");
  CPPUNIT_ASSERT(result == true);

  // Static variable

  m_application->sendShellCommand("setenv PWD /dev");
  m_application->waitShellResponse();

  m_application->sendShellCommand("getenv PWD");
  response = m_application->waitShellResponse();
  result = TestApplication::responseContainsText(response, "/dev");
  CPPUNIT_ASSERT(result == true);
}

CPPUNIT_TEST_SUITE_REGISTRATION(EnvCommandsTest);

int main(int, char *[])
{
  CPPUNIT_NS::Test * const suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();

  CppUnit::TextUi::TestRunner runner;
  runner.addTest(suite);

  const bool sucessful = runner.run();
  return sucessful ? EXIT_SUCCESS : EXIT_FAILURE;
}
