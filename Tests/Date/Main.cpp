/*
 * Main.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "TestApplication.hpp"
#include "Shell/Scripts/DateScript.hpp"
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

class TestDateApplication: public TestApplication
{
public:
  TestDateApplication(Interface *client, Interface *host) :
    TestApplication{client, host}
  {
  }

  void bootstrap() override
  {
    TestApplication::bootstrap();

    m_initializer.attach<DateScript>();
    m_initializer.attach<GetEnvScript>();
  }
};

class DateTest: public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(DateTest);
  CPPUNIT_TEST(testErrorIncorrectAlarm);
  CPPUNIT_TEST(testErrorIncorrectTime);
  CPPUNIT_TEST(testErrorNoAlarmSet);
  CPPUNIT_TEST(testGetTime);
  CPPUNIT_TEST(testHelpMessage);
  CPPUNIT_TEST(testSetAlarm);
  CPPUNIT_TEST(testSetTime);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testErrorIncorrectAlarm();
  void testErrorIncorrectTime();
  void testErrorNoAlarmSet();
  void testGetTime();
  void testHelpMessage();
  void testSetAlarm();
  void testSetTime();

private:
  uv_loop_t *m_loop{nullptr};
  Interrupt *m_listener{nullptr};
  Interface *m_appInterface{nullptr};
  Interface *m_testInterface{nullptr};
  TestApplication *m_application{nullptr};

  std::thread *m_appThread{nullptr};
  std::thread *m_loopThread{nullptr};
};

void DateTest::setUp()
{
  m_loop = uv_default_loop();
  CPPUNIT_ASSERT(m_loop != nullptr);

  m_listener = TestApplication::makeSignalListener(SIGUSR1, onSignalReceived, m_loop);
  CPPUNIT_ASSERT(m_listener != nullptr);
  m_appInterface = TestApplication::makeUdpInterface("127.0.0.1", 8000, 8001);
  CPPUNIT_ASSERT(m_appInterface != nullptr);
  m_testInterface = TestApplication::makeUdpInterface("127.0.0.1", 8001, 8000);
  CPPUNIT_ASSERT(m_testInterface != nullptr);

  m_application = new TestDateApplication(m_appInterface, m_testInterface);
  CPPUNIT_ASSERT(m_application != nullptr);

  m_loopThread = new std::thread{TestApplication::runEventLoop, m_loop};
  CPPUNIT_ASSERT(m_loopThread != nullptr);
  m_appThread = new std::thread{TestApplication::runShell, m_application};
  CPPUNIT_ASSERT(m_appThread != nullptr);

  m_application->waitShellResponse();
}

void DateTest::tearDown()
{
  m_application->sendShellCommand("exit");

  m_appThread->join();
  delete m_appThread;

  m_loopThread->join();
  delete m_loopThread;
}

void DateTest::testErrorIncorrectAlarm()
{
  m_application->sendShellCommand("date -a \"00:00 06.01.1990\"");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "incorrect format");
  CPPUNIT_ASSERT(result == true);

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_VALUE));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void DateTest::testErrorIncorrectTime()
{
  m_application->sendShellCommand("date -s \"06.01.1990 00:00:00\"");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "incorrect format");
  CPPUNIT_ASSERT(result == true);

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_VALUE));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void DateTest::testErrorNoAlarmSet()
{
  m_application->sendShellCommand("date --alarm");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "alarm is not set");
  CPPUNIT_ASSERT(result == true);

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_INVALID));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void DateTest::testGetTime()
{
  m_application->sendShellCommand("date");
  const auto response = m_application->waitShellResponse();

  const auto result = TestApplication::responseContainsText(response, "00:00:00 01.01.1970");
  CPPUNIT_ASSERT(result == true);
}

void DateTest::testHelpMessage()
{
  m_application->sendShellCommand("date --help");
  const auto response = m_application->waitShellResponse();

  const auto result = TestApplication::responseContainsText(response, "Usage");
  CPPUNIT_ASSERT(result == true);
}

void DateTest::testSetAlarm()
{
  m_application->sendShellCommand("date -a \"00:59:59 01.01.1990\"");
  m_application->waitShellResponse();

  m_application->sendShellCommand("date --alarm");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "00:59:59 01.01.1990");
  CPPUNIT_ASSERT(result == true);
}

void DateTest::testSetTime()
{
  m_application->sendShellCommand("date -s \"00:00:00 06.01.1980\"");
  m_application->waitShellResponse();

  m_application->sendShellCommand("date");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "00:00:00 06.01.1980");
  CPPUNIT_ASSERT(result == true);
}

CPPUNIT_TEST_SUITE_REGISTRATION(DateTest);

int main(int, char *[])
{
  CPPUNIT_NS::Test * const suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();

  CppUnit::TextUi::TestRunner runner;
  runner.addTest(suite);

  const bool sucessful = runner.run();
  return sucessful ? EXIT_SUCCESS : EXIT_FAILURE;
}
