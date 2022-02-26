/*
 * Main.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "TestApplication.hpp"
#include "Shell/Scripts/GetEnvScript.hpp"
#include "Shell/Scripts/PrintRawDataScript.hpp"
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

class TestPrintRawDataApplication: public TestApplication
{
public:
  TestPrintRawDataApplication(Interface *client, Interface *host) :
    TestApplication{client, host}
  {
  }

  void bootstrap() override
  {
    TestApplication::bootstrap();

    m_initializer.attach<GetEnvScript>();
    m_initializer.attach<PrintRawDataScript<BUFFER_SIZE>>();
  }
};

class PrintRawDataTest: public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(PrintRawDataTest);
  CPPUNIT_TEST(testDataPrinting);
  CPPUNIT_TEST(testErrorNoNode);
  CPPUNIT_TEST(testHelpMessage);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testDataPrinting();
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

void PrintRawDataTest::setUp()
{
  m_loop = uv_default_loop();
  CPPUNIT_ASSERT(m_loop != nullptr);

  m_listener = TestApplication::makeSignalListener(SIGUSR1, onSignalReceived, m_loop);
  CPPUNIT_ASSERT(m_listener != nullptr);
  m_appInterface = TestApplication::makeUdpInterface("127.0.0.1", 8000, 8001);
  CPPUNIT_ASSERT(m_appInterface != nullptr);
  m_testInterface = TestApplication::makeUdpInterface("127.0.0.1", 8001, 8000);
  CPPUNIT_ASSERT(m_testInterface != nullptr);

  m_application = new TestPrintRawDataApplication(m_appInterface, m_testInterface);

  m_application->makeDataNode("/testA.bin", 16, 'A');
  m_application->makeDataNode("/testB.bin", 256, 'z');

  m_loopThread = new std::thread{TestApplication::runEventLoop, m_loop};
  m_appThread = new std::thread{TestApplication::runShell, m_application};

  m_application->waitShellResponse();
}

void PrintRawDataTest::tearDown()
{
  m_application->sendShellCommand("exit");

  m_appThread->join();
  delete m_appThread;

  m_loopThread->join();
  delete m_loopThread;
}

void PrintRawDataTest::testDataPrinting()
{
  m_application->sendShellCommand("cat /testA.bin");
  const auto response0 = m_application->waitShellResponse();
  const auto result0 = TestApplication::responseContainsText(response0, "AAAA");
  CPPUNIT_ASSERT(result0 == true);

  m_application->sendShellCommand("cat /testB.bin");
  const auto response1 = m_application->waitShellResponse();
  const auto result1 = TestApplication::responseContainsText(response1, "zzzz");
  CPPUNIT_ASSERT(result1 == true);
}

void PrintRawDataTest::testErrorNoNode()
{
  m_application->sendShellCommand("cat /undefined");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "node not found");
  CPPUNIT_ASSERT(result == true);

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_ENTRY));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void PrintRawDataTest::testHelpMessage()
{
  m_application->sendShellCommand("cat --help");
  const auto response = m_application->waitShellResponse();

  const auto result = TestApplication::responseContainsText(response, "Usage");
  CPPUNIT_ASSERT(result == true);
}

CPPUNIT_TEST_SUITE_REGISTRATION(PrintRawDataTest);

int main(int, char *[])
{
  CPPUNIT_NS::Test * const suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();

  CppUnit::TextUi::TestRunner runner;
  runner.addTest(suite);

  const bool sucessful = runner.run();
  return sucessful ? EXIT_SUCCESS : EXIT_FAILURE;
}
