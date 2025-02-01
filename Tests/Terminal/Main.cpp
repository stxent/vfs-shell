/*
 * Main.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "MockTerminal.hpp"
#include "Shell/ShellHelpers.hpp"
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

class TerminalTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(TerminalTest);
  CPPUNIT_TEST(testFontColor);
  CPPUNIT_TEST(testFontStyle);
  CPPUNIT_TEST(testMockTerminal);
  CPPUNIT_TEST(testResultSerializer);
  CPPUNIT_TEST(testShortSerialization);
  CPPUNIT_TEST(testUnsignedShortSerialization);
  CPPUNIT_TEST(testIntSerialization);
  CPPUNIT_TEST(testUnsignedIntSerialization);
  CPPUNIT_TEST(testLongSerialization);
  CPPUNIT_TEST(testUnsignedLongSerialization);
  CPPUNIT_TEST(testLongLongSerialization);
  CPPUNIT_TEST(testUnsignedLongLongSerialization);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testFontColor();
  void testFontStyle();
  void testMockTerminal();
  void testResultSerializer();

  void testShortSerialization()
  {
    testSerializationImpl<short>();
  }

  void testUnsignedShortSerialization()
  {
    testSerializationImpl<unsigned short>();
  }

  void testIntSerialization()
  {
    testSerializationImpl<int>();
  }

  void testUnsignedIntSerialization()
  {
    testSerializationImpl<unsigned int>();
  }

  void testLongSerialization()
  {
    testSerializationImpl<long>();
  }

  void testUnsignedLongSerialization()
  {
    testSerializationImpl<unsigned long>();
  }

  void testLongLongSerialization()
  {
    testSerializationImpl<long long>();
  }

  void testUnsignedLongLongSerialization()
  {
    testSerializationImpl<unsigned long long>();
  }

private:
  static std::string makeColorSeq(Terminal::Color);

  template<typename T>
  void testSerializationImpl();
};

void TerminalTest::setUp()
{
}

void TerminalTest::tearDown()
{
}

void TerminalTest::testFontColor()
{
  MockTerminal terminal{true};
  auto color = Terminal::Color::BLACK;

  while (color <= Terminal::Color::BRIGHT_WHITE)
  {
    std::string output;
    std::string target;

    terminal << color << "test" << Terminal::RESET;
    output = terminal.hostRead();
    target = makeColorSeq(color) + "test\033[0m";
    CPPUNIT_ASSERT(output == target);

    color = static_cast<Terminal::Color>(static_cast<int>(color) + 1);
  }
}

void TerminalTest::testFontStyle()
{
  MockTerminal terminal{true};
  std::string output;
  std::string target;

  terminal << Terminal::BOLD << "test" << Terminal::RESET;
  output = terminal.hostRead();
  target = "\033[1mtest\033[0m";
  CPPUNIT_ASSERT(output == target);
}

void TerminalTest::testMockTerminal()
{
  // For future use
  const std::string target = "test";
  MockTerminal terminal{};

  terminal.subscribe(nullptr);
  terminal.unsubscribe(nullptr);

  std::string input;
  size_t count;
  char buffer[8];

  terminal.hostWrite(target);
  count = terminal.read(buffer, sizeof(buffer));
  input = std::string{buffer, count};

  std::cout << input << " " << count << std::endl;

  CPPUNIT_ASSERT(count == 4);
  CPPUNIT_ASSERT(input == target);
}

void TerminalTest::testResultSerializer()
{
  MockTerminal terminal{true};
  std::string output;
  std::string target;

  terminal << ShellHelpers::ResultSerializer{E_RESULT_END};
  output = terminal.hostRead();
  target = std::to_string(E_RESULT_END);
  CPPUNIT_ASSERT(output == target);
}

std::string TerminalTest::makeColorSeq(Terminal::Color color)
{
  const char high = color >= Terminal::Color::BRIGHT_BLACK ? '9' : '3';
  char low = static_cast<char>('0' + static_cast<int>(color));

  if (color >= Terminal::Color::BRIGHT_BLACK)
    low -= static_cast<int>(Terminal::Color::BRIGHT_BLACK);

  std::string output = {"\033[__m"};

  output[2] = high;
  output[3] = low;

  return output;
}

template<typename T>
void TerminalTest::testSerializationImpl()
{
  MockTerminal terminal{};
  std::string output;
  std::string target;
  T value;

  value = 0;
  terminal << value;
  output = terminal.hostRead();
  target = std::to_string(value);
  CPPUNIT_ASSERT(output == target);

  value = std::numeric_limits<T>::min() / 2;
  terminal << value;
  output = terminal.hostRead();
  target = std::to_string(value);
  CPPUNIT_ASSERT(output == target);

  value = std::numeric_limits<T>::min();
  terminal << value;
  output = terminal.hostRead();
  target = std::to_string(value);
  CPPUNIT_ASSERT(output == target);

  value = std::numeric_limits<T>::max();
  terminal << value;
  output = terminal.hostRead();
  target = std::to_string(value);
  CPPUNIT_ASSERT(output == target);
}

CPPUNIT_TEST_SUITE_REGISTRATION(TerminalTest);

int main(int, char *[])
{
  CPPUNIT_NS::Test * const suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();

  CppUnit::TextUi::TestRunner runner;
  runner.addTest(suite);

  const bool sucessful = runner.run();
  return sucessful ? EXIT_SUCCESS : EXIT_FAILURE;
}
