/*
 * Main.cpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <cassert>
#include "Shell/ArgParser.hpp"

#undef NDEBUG

void testArgumentCounter()
{
  struct Arguments
  {
    Arguments() :
      counter{false}
    {
    }

    size_t counter;

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
  assert(arguments0.counter == 4);

  static const std::array<const char *, 0> input1{{}};
  const auto arguments1 = ArgParser::parse<Arguments>(input1.cbegin(), input1.cend(),
      descriptors.cbegin(), descriptors.cend());
  assert(arguments1.counter == 0);
}

void testMixedArguments()
{
  struct Arguments
  {
    Arguments() :
      counter{false}
    {
    }

    size_t counter;

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
  assert(arguments0.counter == 4);

  static const std::array<const char *, 0> input1{{}};
  const auto arguments1 = ArgParser::parse<Arguments>(input1.cbegin(), input1.cend(),
      descriptors.cbegin(), descriptors.cend());
  assert(arguments1.counter == 0);
}

void testOptionalBool()
{
  struct Arguments
  {
    Arguments() :
      option{false}
    {
    }

    bool option;

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
  assert(correctArguments.option);

  static const std::array<const char *, 1> incorrectInput{{
          "-x"
  }};
  const auto incorrectArguments = ArgParser::parse<Arguments>(incorrectInput.cbegin(), incorrectInput.cend(),
      descriptors.cbegin(), descriptors.cend());
  assert(!incorrectArguments.option);
}

void testOptionalInt()
{
  struct Arguments
  {
    Arguments() :
      option{1} // Non-zero default value
    {
    }

    int option;

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
  assert(correctPosArguments.option == 42);

  static const std::array<const char *, 2> correctNegInput{{
          "-o",
          "-42"
  }};
  const auto correctNegArguments = ArgParser::parse<Arguments>(correctNegInput.cbegin(), correctNegInput.cend(),
      descriptors.cbegin(), descriptors.cend());
  assert(correctNegArguments.option == -42);

  static const std::array<const char *, 1> incorrectInput0{{
          "-o"
  }};
  const auto incorrectArguments0 = ArgParser::parse<Arguments>(incorrectInput0.cbegin(), incorrectInput0.cend(),
      descriptors.cbegin(), descriptors.cend());
  assert(incorrectArguments0.option == 1);

  static const std::array<const char *, 2> incorrectInput1{{
          "-o",
          "none"
  }};
  const auto incorrectArguments1 = ArgParser::parse<Arguments>(incorrectInput1.cbegin(), incorrectInput1.cend(),
      descriptors.cbegin(), descriptors.cend());
  assert(incorrectArguments1.option == 0); // FIXME

  static const std::array<const char *, 1> incorrectInput2{{
          "-x"
  }};
  const auto incorrectArguments2 = ArgParser::parse<Arguments>(incorrectInput2.cbegin(), incorrectInput2.cend(),
      descriptors.cbegin(), descriptors.cend());
  assert(incorrectArguments2.option == 1);
}

int main(int, char *[])
{
  testArgumentCounter();
  testOptionalBool();
  testOptionalInt();
  return EXIT_SUCCESS;
}
