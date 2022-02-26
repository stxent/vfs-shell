/*
 * Environment.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Shell/Environment.hpp"
#include <algorithm>

EnvironmentVariable::operator char *()
{
  return getDataBuffer();
}

EnvironmentVariable::operator const char *() const
{
  return getDataBuffer();
}

void EnvironmentVariable::operator=(const char *buffer)
{
  const size_t requiredLength = strlen(buffer) + 1;
  const size_t availableLength = allocateDataBuffer(requiredLength);
  const size_t endOfString = std::min(availableLength, requiredLength) - 1;

  memcpy(getDataBuffer(), buffer, endOfString);
  getDataBuffer()[endOfString] = '\0';
}

void EnvironmentVariable::operator delete(void *pointer)
{
  free(pointer);
}

DynamicEnvironmentVariable::DynamicEnvironmentVariable(const char *name) :
  m_dataLength{0},
  m_nameBuffer{std::make_unique<char []>(strlen(name) + 1)}
{
  strcpy(m_nameBuffer.get(), name);
  allocateDataBuffer(1);
}

const char *DynamicEnvironmentVariable::getName() const
{
  return m_nameBuffer.get();
}

size_t DynamicEnvironmentVariable::allocateDataBuffer(size_t length)
{
  if (m_dataLength < length)
  {
    m_dataLength = length;
    m_dataBuffer = std::make_unique<char []>(m_dataLength);
    memset(m_dataBuffer.get(), '\0', m_dataLength);
  }

  return m_dataLength;
}

char *DynamicEnvironmentVariable::getDataBuffer()
{
  return m_dataBuffer.get();
}

const char *DynamicEnvironmentVariable::getDataBuffer() const
{
  return m_dataBuffer.get();
}

Environment::Environment() :
  empty{""},
  variables{}
{
}

EnvironmentVariable &Environment::operator[](const char *name)
{
  return make<DynamicEnvironmentVariable>(name);
}

void Environment::iterate(std::function<void (const char *, const char *)> callback)
{
  for (auto iter = variables.begin(); iter != variables.end(); ++iter)
    callback((*iter)->getName(), **iter);
}

void Environment::purge(const char *name)
{
  auto iter = find(name);

  if (iter != variables.end())
    variables.erase(iter);
}
