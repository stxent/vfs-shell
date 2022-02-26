/*
 * Main.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Shell/Environment.hpp"
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

extern "C" void *__libc_malloc(size_t);

static unsigned int mallocHookFails = 0;

extern "C" void *malloc(size_t size)
{
  bool allocate = true;

  if (mallocHookFails && !--mallocHookFails)
    allocate = false;

  return allocate ? __libc_malloc(size) : nullptr;
}

class EnvTest: public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(EnvTest);
  CPPUNIT_TEST(testVariableFailure);
  CPPUNIT_TEST(testVariablePurge);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testVariableFailure();
  void testVariablePurge();
};

void EnvTest::setUp()
{
}

void EnvTest::tearDown()
{
}

void EnvTest::testVariableFailure()
{
  Environment env{};
  std::string output;

  mallocHookFails = 1;
  env["TEST"] = "test";
  output = env["TEST"];
  CPPUNIT_ASSERT(output == "");
}

void EnvTest::testVariablePurge()
{
  Environment env{};
  std::string output;

  env["TEST"] = "test";
  output = env["TEST"];
  CPPUNIT_ASSERT(output == "test");

  env.purge("TEST");
  output = env["TEST"];
  CPPUNIT_ASSERT(output == "");
}

CPPUNIT_TEST_SUITE_REGISTRATION(EnvTest);

int main(int, char *[])
{
  CPPUNIT_NS::Test * const suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();

  CppUnit::TextUi::TestRunner runner;
  runner.addTest(suite);

  const bool sucessful = runner.run();
  return sucessful ? EXIT_SUCCESS : EXIT_FAILURE;
}
