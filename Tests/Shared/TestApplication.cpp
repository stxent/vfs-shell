/*
 * TestApplication.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "TestApplication.hpp"
#include "Shell/MockTimeProvider.hpp"
#include "Shell/Scripts/ExitScript.hpp"
#include "Shell/Scripts/Shell.hpp"
#include "Vfs/VfsDataNode.hpp"
#include "Vfs/VfsHandle.hpp"
#include <halm/platform/generic/signal_handler.h>
#include <halm/platform/generic/udp.h>
#include <cppunit/extensions/HelperMacros.h>
#include <signal.h>
#include <uv.h>

TestApplication::TestApplication(Interface *client, Interface *host, bool echo) :
  m_caller{},
  m_client{client},
  m_host{host},
  m_filesystem{static_cast<FsHandle *>(init(VfsHandleClass, nullptr)), [](FsHandle *pointer){ deinit(pointer); }},
  m_terminal{m_client},
  m_initializer{m_filesystem.get(), m_terminal, MockTimeProvider::instance(), echo}
{
  CPPUNIT_ASSERT(m_client != nullptr);
  CPPUNIT_ASSERT(m_host != nullptr);
  CPPUNIT_ASSERT(m_filesystem != nullptr);
}

TestApplication::SignalCaller::~SignalCaller()
{
  raise(SIGUSR1);
}

void TestApplication::injectNode(const char *path, VfsNode *object)
{
  // Open base node

  FsNode * const parent = fsOpenBaseNode(m_filesystem.get(), path);
  CPPUNIT_ASSERT(parent != nullptr);

  // Inject new node

  const std::array<FsFieldDescriptor, 1> entryFields = {{
      {&object, sizeof(object), static_cast<FsFieldType>(VfsNode::VFS_NODE_OBJECT)}
  }};

  const auto res = fsNodeCreate(parent, entryFields.data(), entryFields.size());
  CPPUNIT_ASSERT(res == E_OK);
  fsNodeFree(parent);
}

void TestApplication::makeDataNode(const char *path, size_t length, char fill)
{
  Result res;

  const char * const name = fsExtractName(path);
  CPPUNIT_ASSERT(name != nullptr);

  // Open base node

  FsNode * const parent = fsOpenBaseNode(m_filesystem.get(), path);
  CPPUNIT_ASSERT(parent != nullptr);

  // Create and open a new node

  std::array<FsFieldDescriptor, 2> desc = {{
      {
          name,
          strlen(name) + 1,
          FS_NODE_NAME
      }, {
          0,
          0,
          FS_NODE_DATA
      }
  }};
  res = fsNodeCreate(parent, desc.data(), desc.size());
  CPPUNIT_ASSERT(res == E_OK);
  fsNodeFree(parent);

  FsNode * const node = fsOpenNode(m_filesystem.get(), path);
  CPPUNIT_ASSERT(node);

  // Write data

  char buffer[BUFFER_SIZE];
  FsLength left = length;
  FsLength position = 0;

  memset(buffer, fill, sizeof(buffer));

  while (left)
  {
    const size_t chunk = MIN((size_t)left, sizeof(buffer));
    size_t count = 0;

    res = fsNodeWrite(node, FS_NODE_DATA, position, buffer, chunk, &count);

    CPPUNIT_ASSERT(res == E_OK);
    CPPUNIT_ASSERT(count == chunk);

    position += chunk;
    left -= (FsLength)chunk;
  }

  FsLength count = 0;
  res = fsNodeLength(node, FS_NODE_DATA, &count);

  CPPUNIT_ASSERT(res == E_OK);
  CPPUNIT_ASSERT(count == length);

  fsNodeFree(node);
}

void TestApplication::makeDataNode(const char *path, const char *buffer, size_t length)
{
  const char * const name = fsExtractName(path);
  CPPUNIT_ASSERT(name != nullptr);

  auto node = new VfsDataNode{name};
  CPPUNIT_ASSERT(node != nullptr);

  const auto result = node->reserve(buffer, length);
  CPPUNIT_ASSERT(result == true);

  injectNode(path, node);
}

void TestApplication::makeDataNode(const char *path, const char *buffer)
{
  makeDataNode(path, buffer, strlen(buffer));
}

void TestApplication::sendShellBuffer(const char *buffer, size_t length)
{
  ifWrite(m_host, buffer, length);
}

void TestApplication::sendShellCommand(const char *command)
{
  ifWrite(m_host, command, strlen(command));
  ifWrite(m_host, "\r\n", 2);
}

void TestApplication::sendShellText(const char *text)
{
  ifWrite(m_host, text, strlen(text));
}

std::vector<std::string> TestApplication::waitShellResponse(std::chrono::milliseconds timeout)
{
  enum class PromptState : uint8_t
  {
    SYMBOL_0,
    SYMBOL_1,
    DONE
  };

  const auto endtime = std::chrono::steady_clock::now() + timeout;
  std::vector<std::string> response;
  std::string line;
  char text[1536];
  PromptState prompt{PromptState::SYMBOL_0};

  line.reserve(1536);

  while (std::chrono::steady_clock::now() <= endtime)
  {
    const size_t count = ifRead(m_host, text, sizeof(text));

    for (size_t i = 0; i < count; ++i)
    {
      char c = text[i];

      switch (prompt)
      {
        case PromptState::SYMBOL_0:
          if (c == '>')
            prompt = PromptState::SYMBOL_1;
          break;

        case PromptState::SYMBOL_1:
          if (c == ' ')
            prompt = PromptState::DONE;
          else
            prompt = PromptState::SYMBOL_0;
          break;

        default:
          break;
      }

      if (c == '\n')
      {
        response.push_back(line);
        line.clear();
      }
      else if (c != '\r')
      {
        line += c;
      }
    }

    if (prompt == PromptState::DONE)
      break;

    usleep(1000);
  }

  if (line.size() > 0)
  {
    response.push_back(line);
    line.clear();
  }

  // TODO Remove
  for (const auto &entry : response)
    std::cout << entry.data() << std::endl;

  return response;
}

Interrupt *TestApplication::makeSignalListener(int signal, void (*callback)(void *), void *argument)
{
  const SignalHandlerConfig config{signal};

  Interrupt * const listener = static_cast<Interrupt *>(init(SignalHandler, &config));
  CPPUNIT_ASSERT(listener != nullptr);

  interruptSetCallback(listener, callback, argument);
  interruptEnable(listener);

  return listener;
}

Interface *TestApplication::makeUdpInterface(const char *ip, uint16_t in, uint16_t out)
{
  const UdpConfig config{ip, out, in};

  Interface * const interface = static_cast<Interface *>(init(Udp, &config));
  CPPUNIT_ASSERT(interface != nullptr);

  return interface;
}

bool TestApplication::responseContainsText(const std::vector<std::string> &response, const std::string &text)
{
  for (const auto &entry : response)
  {
    if (entry.find(text) != std::string::npos)
      return true;
  }

  return false;
}

void TestApplication::runEventLoop(void *argument)
{
  const auto loop = static_cast<uv_loop_t *>(argument);

  uv_run(loop, UV_RUN_DEFAULT);
  uv_loop_close(loop);
}

void TestApplication::runShell(void *argument)
{
  std::unique_ptr<TestApplication> application{static_cast<TestApplication *>(argument)};
  application->run();
}

void TestApplication::bootstrap()
{
  FsNode *parent;

  // Root nodes
  VfsNode *rootEntries[] = {
      new VfsDirectory{"bin", MockTimeProvider::instance().getTime()},
      new VfsDirectory{"dev", MockTimeProvider::instance().getTime()}
  };

  parent = static_cast<FsNode *>(fsHandleRoot(m_filesystem.get()));
  for (auto iter = std::begin(rootEntries); iter != std::end(rootEntries); ++iter)
  {
    const std::array<FsFieldDescriptor, 1> entryFields = {{
        {&*iter, sizeof(*iter), static_cast<FsFieldType>(VfsNode::VFS_NODE_OBJECT)}
    }};
    fsNodeCreate(parent, entryFields.data(), entryFields.size());
  }
  fsNodeFree(parent);

  m_initializer.attach<ExitScript>();
  m_initializer.attach<Shell>();
}

void TestApplication::run()
{
  bootstrap();
  m_initializer.run();
}
