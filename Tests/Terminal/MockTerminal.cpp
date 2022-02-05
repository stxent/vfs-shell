/*
 * MockTerminal.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "MockTerminal.hpp"
#include "Shell/Script.hpp"
#include <cppunit/extensions/HelperMacros.h>
#include <algorithm>

MockTerminal::MockTerminal(bool coloration) :
  Terminal{coloration}
{
  bool result;

  result = byteQueueInit(&rxQueue, QUEUE_SIZE);
  CPPUNIT_ASSERT(result = true);
  result = byteQueueInit(&txQueue, QUEUE_SIZE);
  CPPUNIT_ASSERT(result = true);
}

MockTerminal::~MockTerminal()
{
  byteQueueDeinit(&txQueue);
  byteQueueDeinit(&rxQueue);
}

void MockTerminal::subscribe(Script *)
{
}

void MockTerminal::unsubscribe(const Script *)
{
}

size_t MockTerminal::read(char *buffer, size_t length)
{
  return byteQueuePopArray(&txQueue, buffer, length);
}

size_t MockTerminal::write(const char *buffer, size_t length)
{
  return byteQueuePushArray(&rxQueue, buffer, length);
}

std::string MockTerminal::hostRead()
{
  std::string output;
  char buffer[BUFFER_SIZE];
  size_t count;

  while ((count = byteQueuePopArray(&rxQueue, buffer, sizeof(buffer))) > 0)
    output += std::string(buffer, count);

  return output;
}

void MockTerminal::hostWrite(const std::string &text)
{
  byteQueuePushArray(&txQueue, text.data(), text.size());
}
