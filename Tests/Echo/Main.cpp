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

class TestEchoApplication: public TestApplication
{
public:
  TestEchoApplication(Interface *client, Interface *host) :
    TestApplication{client, host}
  {
  }

  void bootstrap() override
  {
    TestApplication::bootstrap();

    m_initializer.attach<EchoScript>();
  }
};

class EchoTest: public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(EchoTest);
  CPPUNIT_TEST(testMultiAgrumentOutput);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testMultiAgrumentOutput();

private:
  uv_loop_t *m_loop{nullptr};
  Interrupt *m_listener{nullptr};
  Interface *m_appInterface{nullptr};
  Interface *m_testInterface{nullptr};
  TestApplication *m_application{nullptr};

  std::thread *m_appThread{nullptr};
  std::thread *m_loopThread{nullptr};
};

void EchoTest::setUp()
{
  m_loop = uv_default_loop();
  CPPUNIT_ASSERT(m_loop != nullptr);

  m_listener = TestApplication::makeSignalListener(SIGUSR1, onSignalReceived, m_loop);
  CPPUNIT_ASSERT(m_listener != nullptr);
  m_appInterface = TestApplication::makeUdpInterface("127.0.0.1", 8000, 8001);
  CPPUNIT_ASSERT(m_appInterface != nullptr);
  m_testInterface = TestApplication::makeUdpInterface("127.0.0.1", 8001, 8000);
  CPPUNIT_ASSERT(m_testInterface != nullptr);

  m_application = new TestEchoApplication(m_appInterface, m_testInterface);
  CPPUNIT_ASSERT(m_application != nullptr);

  m_loopThread = new std::thread{TestApplication::runEventLoop, m_loop};
  CPPUNIT_ASSERT(m_loopThread != nullptr);
  m_appThread = new std::thread{TestApplication::runShell, m_application};
  CPPUNIT_ASSERT(m_appThread != nullptr);

  m_application->waitShellResponse();
}

void EchoTest::tearDown()
{
  m_application->sendShellCommand("exit");

  m_appThread->join();
  delete m_appThread;

  m_loopThread->join();
  delete m_loopThread;
}

void EchoTest::testMultiAgrumentOutput()
{
  m_application->sendShellCommand("echo text \"string argument\"");
  const auto response = m_application->waitShellResponse();

  const auto result0 = TestApplication::responseContainsText(response, "text");
  CPPUNIT_ASSERT(result0 == true);
  const auto result1 = TestApplication::responseContainsText(response, "string argument");
  CPPUNIT_ASSERT(result1 == true);
}

CPPUNIT_TEST_SUITE_REGISTRATION(EchoTest);

int main(int, char *[])
{
  CPPUNIT_NS::Test * const suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();

  CppUnit::TextUi::TestRunner runner;
  runner.addTest(suite);

  const bool sucessful = runner.run();
  return sucessful ? EXIT_SUCCESS : EXIT_FAILURE;
}
