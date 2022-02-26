/*
 * Core/Shell/Environment.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_ENVIRONMENT_HPP_
#define VFS_SHELL_CORE_SHELL_ENVIRONMENT_HPP_

#include <cstring>
#include <functional>
#include <memory>
#include <new>
#include <vector>

class EnvironmentVariable
{
public:
  virtual ~EnvironmentVariable() = default;
  virtual const char *getName() const = 0;

  operator char *();
  operator const char *() const;
  void operator=(const char *buffer);
  void operator delete(void *);

protected:
  virtual size_t allocateDataBuffer(size_t) = 0;
  virtual char *getDataBuffer() = 0;
  virtual const char *getDataBuffer() const = 0;
};

class DynamicEnvironmentVariable: public EnvironmentVariable
{
public:
  DynamicEnvironmentVariable(const DynamicEnvironmentVariable &) = delete;
  DynamicEnvironmentVariable &operator=(const DynamicEnvironmentVariable &) = delete;

  DynamicEnvironmentVariable(const char *name);
  virtual const char *getName() const override;

protected:
  virtual size_t allocateDataBuffer(size_t length) override;
  virtual char *getDataBuffer() override;
  virtual const char *getDataBuffer() const override;

private:
  size_t m_dataLength;
  std::unique_ptr<char []> m_dataBuffer;
  std::unique_ptr<char []> m_nameBuffer;
};

template<size_t LENGTH>
class StaticEnvironmentVariable: public EnvironmentVariable
{
public:
  StaticEnvironmentVariable(const StaticEnvironmentVariable<LENGTH> &) = delete;
  StaticEnvironmentVariable<LENGTH> &operator=(const StaticEnvironmentVariable<LENGTH> &) = delete;

  StaticEnvironmentVariable(const char *name) :
    m_dataBuffer{'\0'},
    m_nameBuffer{std::make_unique<char []>(strlen(name) + 1)}
  {
    strcpy(m_nameBuffer.get(), name);
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

  virtual const char *getDataBuffer() const override
  {
    return m_dataBuffer;
  }

private:
  char m_dataBuffer[LENGTH];
  std::unique_ptr<char []> m_nameBuffer;
};

class Environment
{
  StaticEnvironmentVariable<1> empty;
  std::vector<std::unique_ptr<EnvironmentVariable>> variables;

  auto find(const char *name)
  {
    return std::find_if(variables.begin(), variables.end(),
        [name](std::unique_ptr<EnvironmentVariable> &entry){ return strcmp(entry->getName(), name) == 0; });
  }

public:
  Environment();

  EnvironmentVariable &operator[](const char *);
  void iterate(std::function<void (const char *, const char *)>);
  void purge(const char *);

  template<typename T>
  EnvironmentVariable &make(const char *name)
  {
    auto iter = find(name);

    if (iter == variables.end())
    {
      const auto variable = static_cast<T *>(malloc(sizeof(T)));

      if (variable != nullptr)
      {
        new (variable) T{name};
        variables.emplace_back(variable);
        return *variable;
      }
      else
        return empty;
    }

    return *(*iter).get();
  }
};

#endif // VFS_SHELL_CORE_SHELL_ENVIRONMENT_HPP_
