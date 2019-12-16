/*
 * Core/Shell/Environment.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_ENVIRONMENT_HPP_
#define VFS_SHELL_CORE_SHELL_ENVIRONMENT_HPP_

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <functional>
#include <memory>
#include <vector>

class EnvironmentVariable
{
public:
  virtual ~EnvironmentVariable() = default;
  virtual size_t getLength() const = 0;
  virtual const char *getName() const = 0;

  operator char *()
  {
    return getDataBuffer();
  }

  operator const char *() const
  {
    return getDataBuffer();
  }

  void operator=(const char *buffer)
  {
    const size_t requiredLength = strlen(buffer) + 1;
    const size_t availableLength = allocateDataBuffer(requiredLength);
    const size_t endOfString = std::min(availableLength, requiredLength) - 1;

    memcpy(getDataBuffer(), buffer, endOfString);
    getDataBuffer()[endOfString] = '\0';
  }

protected:
  virtual size_t allocateDataBuffer(size_t) = 0;
  virtual char *getDataBuffer() = 0;
  virtual const char *getDataBuffer() const = 0;
  // TODO Add static for an empty string
};

class DynamicEnvironmentVariable: public EnvironmentVariable
{
public:
  DynamicEnvironmentVariable(const char *name) :
    m_dataLength{0},
    m_nameBuffer{std::make_unique<char []>(strlen(name) + 1)}
  {
    strcpy(m_nameBuffer.get(), name);
    allocateDataBuffer(1);
  }

  virtual size_t getLength() const override
  {
    return m_dataLength;
  }

  virtual const char *getName() const override
  {
    return m_nameBuffer.get();
  }

protected:
  virtual size_t allocateDataBuffer(size_t length) override
  {
    if (m_dataLength < length)
    {
      m_dataLength = length;
      m_dataBuffer = std::make_unique<char []>(m_dataLength);
      memset(m_dataBuffer.get(), '\0', m_dataLength);
    }

    return m_dataLength;
  }

  virtual char *getDataBuffer() override
  {
    return m_dataBuffer.get();
  }

  virtual const char *getDataBuffer() const
  {
    return m_dataBuffer.get();
  }

private:
  size_t m_dataLength;
  std::unique_ptr<char []> m_dataBuffer;
  std::unique_ptr<char []> m_nameBuffer;
};

template<size_t LENGTH>
class StaticEnvironmentVariable: public EnvironmentVariable
{
public:
  StaticEnvironmentVariable(const char *name) :
    m_dataBuffer{'\0'},
    m_nameBuffer{std::make_unique<char []>(strlen(name) + 1)}
  {
    strcpy(m_nameBuffer.get(), name);
  }

  virtual size_t getLength() const override
  {
    return LENGTH;
  }

  virtual const char *getName() const override
  {
    return m_nameBuffer.get();
  }

protected:
  virtual size_t allocateDataBuffer(size_t) override
  {
    return LENGTH;
  }

  virtual char *getDataBuffer() override
  {
    return m_dataBuffer;
  }

  virtual const char *getDataBuffer() const
  {
    return m_dataBuffer;
  }

private:
  char m_dataBuffer[LENGTH];
  std::unique_ptr<char []> m_nameBuffer;
};

class Environment
{
  std::vector<std::unique_ptr<EnvironmentVariable>> variables;

  auto find(const char *name)
  {
    return std::find_if(variables.begin(), variables.end(),
        [name](std::unique_ptr<EnvironmentVariable> &entry){ return strcmp(entry->getName(), name) == 0; });
  }

public:
  EnvironmentVariable &operator[](const char *name)
  {
    // TODO Mutex
    auto iter = find(name);

    if (iter == variables.end())
    {
      EnvironmentVariable * const variable = new DynamicEnvironmentVariable{name};
      variables.emplace_back(variable);
      return *variable;
    }
    else
      return *(*iter).get();
  }

  void iterate(std::function<void (const char *, const char *)> callback)
  {
    for (auto iter = variables.begin(); iter != variables.end(); ++iter)
      callback((*iter)->getName(), **iter);
  }

  template<typename T>
  EnvironmentVariable &make(const char *name)
  {
    auto iter = find(name);

    if (iter == variables.end())
    {
      EnvironmentVariable * const variable = new T{name};
      variables.emplace_back(variable);
      return *variable;
    }
    else
      return *(*iter).get();
  }

  void purge(const char *name)
  {
    auto iter = find(name);

    if (iter != variables.end())
      variables.erase(iter);
  }
};

#endif // VFS_SHELL_CORE_SHELL_ENVIRONMENT_HPP_
