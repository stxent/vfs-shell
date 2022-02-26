/*
 * Main.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Shell/ArgParser.hpp"
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

class ArgParserTest: public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(ArgParserTest);
  CPPUNIT_TEST(testArgumentCounter);
  CPPUNIT_TEST(testOptionalBool);
  CPPUNIT_TEST(testOptionalInt);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testArgumentCounter();
  void testOptionalBool();
  void testOptionalInt();
};

void ArgParserTest::setUp()
{
}

void ArgParserTest::tearDown()
{
}

void ArgParserTest::testArgumentCounter()
{
  struct Arguments
  {
    size_t counter{0};

    static void entryHandler(void *object, const char *)
    {
      ++static_cast<Arguments *>(object)->counter;
    }
  };

  static const std::array<ArgParser::Descriptor, 1> descriptors{{
          {nullptr, nullptr, "entries", 0, Arguments::entryHandler}
  }};

  static const std::array<const char *, 4> input0{{
          "a",
          "b",
          "c",
          "d"
  }};
  const auto arguments0 = ArgParser::parse<Arguments>(input0.cbegin(), input0.cend(),
      descriptors.cbegin(), descriptors.cend());
  CPPUNIT_ASSERT(arguments0.counter == 4);

  static const std::array<const char *, 0> input1{{}};
  const auto arguments1 = ArgParser::parse<Arguments>(input1.cbegin(), input1.cend(),
      descriptors.cbegin(), descriptors.cend());
  CPPUNIT_ASSERT(arguments1.counter == 0);
}

void ArgParserTest::testOptionalBool()
{
  struct Arguments
  {
    bool option{false};

    static void optionSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->option = true;
    }
  };

  static const std::array<ArgParser::Descriptor, 1> descriptors{{
          {"-o", nullptr, "option", 0, Arguments::optionSetter}
  }};

  static const std::array<const char *, 1> correctInput{{
          "-o"
  }};
  const auto correctArguments = ArgParser::parse<Arguments>(correctInput.cbegin(), correctInput.cend(),
      descriptors.cbegin(), descriptors.cend());
  CPPUNIT_ASSERT(correctArguments.option);

  static const std::array<const char *, 1> incorrectInput{{
          "-x"
  }};
  const auto incorrectArguments = ArgParser::parse<Arguments>(incorrectInput.cbegin(), incorrectInput.cend(),
      descriptors.cbegin(), descriptors.cend());
  CPPUNIT_ASSERT(!incorrectArguments.option);
}

void ArgParserTest::testOptionalInt()
{
  struct Arguments
  {
    int option{1}; // Non-zero default value

    static void optionSetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->option = atoi(argument);
    }
  };

  static const std::array<ArgParser::Descriptor, 1> descriptors{{
          {"-o", nullptr, "option", 1, Arguments::optionSetter}
  }};

  static const std::array<const char *, 2> correctPosInput{{
          "-o",
          "42"
  }};
  const auto correctPosArguments = ArgParser::parse<Arguments>(correctPosInput.cbegin(), correctPosInput.cend(),
      descriptors.cbegin(), descriptors.cend());
  CPPUNIT_ASSERT(correctPosArguments.option == 42);

  static const std::array<const char *, 2> correctNegInput{{
          "-o",
          "-42"
  }};
  const auto correctNegArguments = ArgParser::parse<Arguments>(correctNegInput.cbegin(), correctNegInput.cend(),
      descriptors.cbegin(), descriptors.cend());
  CPPUNIT_ASSERT(correctNegArguments.option == -42);

  static const std::array<const char *, 1> incorrectInput0{{
          "-o"
  }};
  const auto incorrectArguments0 = ArgParser::parse<Arguments>(incorrectInput0.cbegin(), incorrectInput0.cend(),
      descriptors.cbegin(), descriptors.cend());
  CPPUNIT_ASSERT(incorrectArguments0.option == 1);

  ArgParser::invoke(incorrectInput0.cbegin(), incorrectInput0.cend(), descriptors.cbegin(), descriptors.cend(),
      [this](const char *){});

  static const std::array<const char *, 2> incorrectInput1{{
          "-o",
          "none"
  }};
  const auto incorrectArguments1 = ArgParser::parse<Arguments>(incorrectInput1.cbegin(), incorrectInput1.cend(),
      descriptors.cbegin(), descriptors.cend());
  CPPUNIT_ASSERT(incorrectArguments1.option == 0);

  static const std::array<const char *, 1> incorrectInput2{{
          "-x"
  }};
  const auto incorrectArguments2 = ArgParser::parse<Arguments>(incorrectInput2.cbegin(), incorrectInput2.cend(),
      descriptors.cbegin(), descriptors.cend());
  CPPUNIT_ASSERT(incorrectArguments2.option == 1);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ArgParserTest);

int main(int, char *[])
{
  CPPUNIT_NS::Test * const suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();

  CppUnit::TextUi::TestRunner runner;
  runner.addTest(suite);

  const bool sucessful = runner.run();
  return sucessful ? EXIT_SUCCESS : EXIT_FAILURE;
}
