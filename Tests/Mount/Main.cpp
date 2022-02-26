/*
 * Main.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "TestApplication.hpp"
#include "VirtualMem.hpp"
#include "Shell/Interfaces/InterfaceNode.hpp"
#include "Shell/Scripts/EchoScript.hpp"
#include "Shell/Scripts/GetEnvScript.hpp"
#include "Shell/Scripts/ListNodesScript.hpp"
#include "Shell/Scripts/MountScript.hpp"
#include "Shell/Scripts/RemoveNodesScript.hpp"
#include <yaf/utils.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <uv.h>
#include <thread>

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

    m_initializer.attach<EchoScript>();
    m_initializer.attach<GetEnvScript>();
    m_initializer.attach<ListNodesScript>();
    m_initializer.attach<MountScript<>>();
    m_initializer.attach<RemoveNodesScript>();
  }
};

class MountTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(MountTest);
  CPPUNIT_TEST(testErrorIncorrectArguments);
  CPPUNIT_TEST(testErrorNoDestination);
  CPPUNIT_TEST(testErrorNoInterface);
  CPPUNIT_TEST(testErrorNoNode);
  CPPUNIT_TEST(testErrorUnformatted);
  CPPUNIT_TEST(testHelpMessage);
  CPPUNIT_TEST(testMount);
  CPPUNIT_TEST(testNodes);
  CPPUNIT_TEST(testVirtualMem);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testErrorIncorrectArguments();
  void testErrorNoDestination();
  void testErrorNoInterface();
  void testErrorNoNode();
  void testErrorUnformatted();
  void testHelpMessage();
  void testMount();
  void testNodes();
  void testVirtualMem();

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

void MountTest::testErrorIncorrectArguments()
{
  m_application->sendShellCommand("mount");
  m_application->waitShellResponse();

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_VALUE));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void MountTest::testErrorNoDestination()
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

  m_application->sendShellCommand("mount /dev/mem /undefined/mnt");
  response = m_application->waitShellResponse();
  ok = TestApplication::responseContainsText(response, "mount failed");
  CPPUNIT_ASSERT(ok == true);

  m_application->sendShellCommand("getenv ?");
  response = m_application->waitShellResponse();
  ok = TestApplication::responseContainsText(response, std::to_string(E_OK));
  CPPUNIT_ASSERT(ok == false);
}

void MountTest::testErrorNoInterface()
{
  m_application->sendShellCommand("mount /dev /mnt");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "incorrect device");
  CPPUNIT_ASSERT(result == true);

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_INTERFACE));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void MountTest::testErrorNoNode()
{
  m_application->sendShellCommand("mount /dev/mem /mnt");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "node not found");
  CPPUNIT_ASSERT(result == true);

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_ENTRY));
  CPPUNIT_ASSERT(returnValueFound == true);
}

void MountTest::testErrorUnformatted()
{
  VfsNode * const virtualMemNode = new InterfaceNode<>{m_mem};
  CPPUNIT_ASSERT(virtualMemNode != nullptr);
  m_application->injectNode(virtualMemNode, "/dev/mem");

  m_application->sendShellCommand("mount /dev/mem /mnt");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "mount failed");
  CPPUNIT_ASSERT(result == true);

  m_application->sendShellCommand("getenv ?");
  const auto returnValue = m_application->waitShellResponse();
  const auto returnValueFound = TestApplication::responseContainsText(returnValue, std::to_string(E_OK));
  CPPUNIT_ASSERT(returnValueFound == false);
}

void MountTest::testHelpMessage()
{
  m_application->sendShellCommand("mount --help");
  const auto response = m_application->waitShellResponse();

  const auto result = TestApplication::responseContainsText(response, "Usage");
  CPPUNIT_ASSERT(result == true);
}

void MountTest::testMount()
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

  m_application->sendShellCommand("mount /dev/mem /mnt");
  m_application->waitShellResponse();

  m_application->sendShellCommand("ls /");
  response = m_application->waitShellResponse();
  ok = TestApplication::responseContainsText(response, "mnt");
  CPPUNIT_ASSERT(ok == true);
}

void MountTest::testNodes()
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

  m_application->sendShellCommand("mount /dev/mem /mnt");
  m_application->waitShellResponse();

  m_application->sendShellCommand("echo test > /mnt/TEST.TXT");
  m_application->waitShellResponse();

  m_application->sendShellCommand("ls /mnt");
  response = m_application->waitShellResponse();
  ok = TestApplication::responseContainsText(response, "TEST.TXT");
  CPPUNIT_ASSERT(ok == true);

  m_application->sendShellCommand("rm /mnt/TEST.TXT");
  m_application->waitShellResponse();

  m_application->sendShellCommand("ls /mnt");
  response = m_application->waitShellResponse();
  ok = TestApplication::responseContainsText(response, "node has no descendants");
  CPPUNIT_ASSERT(ok == true);
}

void MountTest::testVirtualMem()
{
  const auto proxy = MockMountInterfaceBuilder::build(m_mem);
  CPPUNIT_ASSERT(proxy != nullptr);

  uint64_t position;
  Result res;

  res = ifGetParam(proxy, IF_STATUS, nullptr);
  CPPUNIT_ASSERT(res == E_OK);

  res = ifSetParam(proxy, IF_STATUS, nullptr);
  CPPUNIT_ASSERT(res == E_INVALID);

  position = PARTITION_SIZE + 1;
  res = ifSetParam(proxy, IF_POSITION_64, &position);
  CPPUNIT_ASSERT(res == E_ADDRESS);

  position = PARTITION_SIZE / 2;
  res = ifSetParam(proxy, IF_POSITION_64, &position);
  CPPUNIT_ASSERT(res == E_OK);

  position = 0;
  res = ifGetParam(proxy, IF_POSITION_64, &position);
  CPPUNIT_ASSERT(res == E_OK);
  CPPUNIT_ASSERT(position == PARTITION_SIZE / 2);

  deinit(proxy);
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
