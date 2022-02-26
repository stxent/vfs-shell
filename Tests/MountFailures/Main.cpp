/*
 * Main.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "TestApplication.hpp"
#include "VirtualMem.hpp"
#include "Shell/Interfaces/InterfaceNode.hpp"
#include "Shell/Scripts/GetEnvScript.hpp"
#include "Shell/Scripts/MountScript.hpp"
#include <yaf/utils.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <uv.h>
#include <thread>

extern "C" void *__libc_malloc(size_t);

static unsigned int mallocHookFails = 0;

extern "C" void *malloc(size_t size)
{
  bool allocate = true;

  if (mallocHookFails)
  {
    // Ignore first waitShellResponse allocation
    if (size < 1024 && !--mallocHookFails)
      allocate = false;
  }

  return allocate ? __libc_malloc(size) : nullptr;
}

static void onUvWalk(uv_handle_t *handle, void *)
{
  deinit(uv_handle_get_data(handle));
}

static void onSignalReceived(void *argument)
{
  uv_walk(static_cast<uv_loop_t *>(argument), onUvWalk, 0);
}

class TestMountApplication: public TestApplication
{
public:
  TestMountApplication(Interface *client, Interface *host) :
    TestApplication{client, host, false}
  {
  }

  void bootstrap() override
  {
    TestApplication::bootstrap();

    m_initializer.attach<GetEnvScript>();
    m_initializer.attach<MountScript<>>();
  }
};

class MountTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(MountTest);
  CPPUNIT_TEST(testMemoryFailures);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testMemoryFailures();

private:
  static constexpr size_t PARTITION_SIZE{1024 * 1024};

  uv_loop_t *m_loop{nullptr};
  Interrupt *m_listener{nullptr};
  Interface *m_appInterface{nullptr};
  Interface *m_testInterface{nullptr};
  Interface *m_mem{nullptr};
  TestApplication *m_application{nullptr};

  std::thread *m_appThread{nullptr};
  std::thread *m_loopThread{nullptr};
};

void MountTest::setUp()
{
  static const VirtualMem::Config virtualMemConfig = {
      PARTITION_SIZE // size
  };

  m_loop = uv_default_loop();
  CPPUNIT_ASSERT(m_loop != nullptr);

  m_listener = TestApplication::makeSignalListener(SIGUSR1, onSignalReceived, m_loop);
  CPPUNIT_ASSERT(m_listener != nullptr);
  m_appInterface = TestApplication::makeUdpInterface("127.0.0.1", 8000, 8001);
  CPPUNIT_ASSERT(m_appInterface != nullptr);
  m_testInterface = TestApplication::makeUdpInterface("127.0.0.1", 8001, 8000);
  CPPUNIT_ASSERT(m_testInterface != nullptr);
  m_mem = static_cast<Interface *>(init(VirtualMem, &virtualMemConfig));
  CPPUNIT_ASSERT(m_mem != nullptr);

  m_application = new TestMountApplication(m_appInterface, m_testInterface);

  m_loopThread = new std::thread{TestApplication::runEventLoop, m_loop};
  m_appThread = new std::thread{TestApplication::runShell, m_application};

  m_application->waitShellResponse();
}

void MountTest::tearDown()
{
  m_application->sendShellCommand("exit");

  m_appThread->join();
  delete m_appThread;

  m_loopThread->join();
  delete m_loopThread;

  deinit(m_mem);
}

void MountTest::testMemoryFailures()
{
  static const Fat32FsConfig makeFsConfig = {
      1024,  // clusterSize
      2,     // tableCount
      "TEST" // label
  };

  std::vector<std::string> response;
  Result res;
  bool ok;

  res = fat32MakeFs(m_mem, &makeFsConfig);
  CPPUNIT_ASSERT(res == E_OK);

  VfsNode * const virtualMemNode = new InterfaceNode<>{m_mem};
  CPPUNIT_ASSERT(virtualMemNode != nullptr);
  m_application->injectNode(virtualMemNode, "/dev/mem");

  // Proxy interface creation failure

  mallocHookFails = 7;
  m_application->sendShellCommand("mount /dev/mem /mnt");
  response = m_application->waitShellResponse();
  ok = TestApplication::responseContainsText(response, "mount failed");
  CPPUNIT_ASSERT(ok == true);

  m_application->sendShellCommand("getenv ?");
  response = m_application->waitShellResponse();
  ok = TestApplication::responseContainsText(response, std::to_string(E_INTERFACE));
  CPPUNIT_ASSERT(ok == true);

  // Mountpoint creation failure

  mallocHookFails = 13;
  m_application->sendShellCommand("mount /dev/mem /mnt");
  response = m_application->waitShellResponse();
  ok = TestApplication::responseContainsText(response, "mount failed");
  CPPUNIT_ASSERT(ok == true);

  m_application->sendShellCommand("getenv ?");
  response = m_application->waitShellResponse();
  ok = TestApplication::responseContainsText(response, std::to_string(E_MEMORY));
  CPPUNIT_ASSERT(ok == true);
}

CPPUNIT_TEST_SUITE_REGISTRATION(MountTest);

int main(int, char *[])
{
  CPPUNIT_NS::Test * const suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();

  CppUnit::TextUi::TestRunner runner;
  runner.addTest(suite);

  const bool sucessful = runner.run();
  return sucessful ? EXIT_SUCCESS : EXIT_FAILURE;
}
