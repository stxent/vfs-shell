/*
 * Main.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "TestApplication.hpp"
#include "Shell/Scripts/DirectDataScript.hpp"
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
  }
};

class DirectDataTest: public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(DirectDataTest);
  CPPUNIT_TEST(testHelpMessage);
  CPPUNIT_TEST(testMinimalNodeCopy);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testHelpMessage();
  void testMinimalNodeCopy();

private:
  uv_loop_t *m_loop{nullptr};
  Interrupt *m_listener{nullptr};
  Interface *m_appInterface{nullptr};
  Interface *m_testInterface{nullptr};
  TestApplication *m_application{nullptr};

  std::thread *m_appThread{nullptr};
  std::thread *m_loopThread{nullptr};
};

void DirectDataTest::setUp()
{
  m_loop = uv_default_loop();

  m_listener = TestApplication::makeSignalListener(SIGUSR1, onSignalReceived, m_loop);
  m_appInterface = TestApplication::makeUdpInterface("127.0.0.1", 8000, 8001);
  m_testInterface = TestApplication::makeUdpInterface("127.0.0.1", 8001, 8000);

  m_application = new TestDirectDataApplication(m_appInterface, m_testInterface);

  m_loopThread = new std::thread{TestApplication::runEventLoop, m_loop};
  m_appThread = new std::thread{TestApplication::runShell, m_application};

  m_application->makeDataNode("/test.bin", 65536, 'A');
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

void DirectDataTest::testHelpMessage()
{
  m_application->sendShellCommand("dd --help");
  const auto response = m_application->waitShellResponse();

  const auto result = TestApplication::responseContainsText(response, "Usage");
  CPPUNIT_ASSERT(result == true);
}

void DirectDataTest::testMinimalNodeCopy()
{
  m_application->sendShellCommand("dd --if /test.bin --of /test_2.bin");
  const auto response = m_application->waitShellResponse();
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
