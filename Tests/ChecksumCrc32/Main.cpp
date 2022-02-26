/*
 * Main.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "TestApplication.hpp"
#include "Shell/Scripts/ChecksumCrc32Script.hpp"
#include "Shell/Scripts/GetEnvScript.hpp"
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

class TestChecksumCrc32Application: public TestApplication
{
public:
  TestChecksumCrc32Application(Interface *client, Interface *host) :
    TestApplication{client, host}
  {
  }

  void bootstrap() override
  {
    TestApplication::bootstrap();

    m_initializer.attach<ChecksumCrc32Script<BUFFER_SIZE>>();
    m_initializer.attach<GetEnvScript>();
  }
};

class ChecksumCrc32Test: public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(ChecksumCrc32Test);
  CPPUNIT_TEST(testChecksumCalc);
  CPPUNIT_TEST(testErrorNoNode);
  CPPUNIT_TEST(testHelpMessage);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testChecksumCalc();
  void testErrorNoNode();
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

void ChecksumCrc32Test::setUp()
{
  m_loop = uv_default_loop();
  CPPUNIT_ASSERT(m_loop != nullptr);

  m_listener = TestApplication::makeSignalListener(SIGUSR1, onSignalReceived, m_loop);
  CPPUNIT_ASSERT(m_listener != nullptr);
  m_appInterface = TestApplication::makeUdpInterface("127.0.0.1", 8000, 8001);
  CPPUNIT_ASSERT(m_appInterface != nullptr);
  m_testInterface = TestApplication::makeUdpInterface("127.0.0.1", 8001, 8000);
  CPPUNIT_ASSERT(m_testInterface != nullptr);

  m_application = new TestChecksumCrc32Application(m_appInterface, m_testInterface);

  m_application->makeDataNode("/test.bin", 65536, 'A');

  m_loopThread = new std::thread{TestApplication::runEventLoop, m_loop};
  m_appThread = new std::thread{TestApplication::runShell, m_application};

  m_application->waitShellResponse();
}

void ChecksumCrc32Test::tearDown()
{
  m_application->sendShellCommand("exit");

  m_appThread->join();
  delete m_appThread;

  m_loopThread->join();
  delete m_loopThread;
}

void ChecksumCrc32Test::testChecksumCalc()
{
  m_application->sendShellCommand("cksum /test.bin");
  const auto response = m_application->waitShellResponse();

  const auto result = TestApplication::responseContainsText(response, "A09B0680");
  CPPUNIT_ASSERT(result == true);
}

void ChecksumCrc32Test::testErrorNoNode()
{
  m_application->sendShellCommand("cksum undefined test.bin");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "open failed");
  CPPUNIT_ASSERT(result == true);

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_ENTRY));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void ChecksumCrc32Test::testHelpMessage()
{
  m_application->sendShellCommand("cksum --help");
  const auto response = m_application->waitShellResponse();

  const auto result = TestApplication::responseContainsText(response, "Usage");
  CPPUNIT_ASSERT(result == true);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ChecksumCrc32Test);

int main(int, char *[])
{
  CPPUNIT_NS::Test * const suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();

  CppUnit::TextUi::TestRunner runner;
  runner.addTest(suite);

  const bool sucessful = runner.run();
  return sucessful ? EXIT_SUCCESS : EXIT_FAILURE;
}
