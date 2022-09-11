/*
 * Main.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "MockDisplay.hpp"
#include "MockInterface.hpp"
#include "MockSerial.hpp"
#include "TestApplication.hpp"
#include "Shell/Interfaces/InterfaceNode.hpp"
#include "Shell/MockTimeProvider.hpp"
#include "Shell/Scripts/EchoScript.hpp"
#include "Shell/Scripts/ListNodesScript.hpp"
#include "Shell/Scripts/PrintRawDataScript.hpp"
#include "Shell/Scripts/RemoveNodesScript.hpp"
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

class TestInterfaceNodeApplication: public TestApplication
{
public:
  TestInterfaceNodeApplication(Interface *client, Interface *host) :
    TestApplication{client, host, false}
  {
  }

  void bootstrap() override
  {
    TestApplication::bootstrap();

    m_initializer.attach<EchoScript>();
    m_initializer.attach<ListNodesScript>();
    m_initializer.attach<PrintRawDataScript<BUFFER_SIZE>>();
    m_initializer.attach<RemoveNodesScript>();
  }
};

class InterfaceNodeTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(InterfaceNodeTest);
  CPPUNIT_TEST(testAccess);
  CPPUNIT_TEST(testDisplayNode);
  CPPUNIT_TEST(testDisplayReadWrite);
  CPPUNIT_TEST(testEmptyNode);
  CPPUNIT_TEST(testInterfaceBase);
  CPPUNIT_TEST(testInterfaceNode);
  CPPUNIT_TEST(testInterfaceNode64);
  CPPUNIT_TEST(testMockDisplay);
  CPPUNIT_TEST(testMockInterface);
  CPPUNIT_TEST(testMockSerial);
  CPPUNIT_TEST(testSerialNode);
  CPPUNIT_TEST(testSubnode);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testAccess();
  void testDisplayNode();
  void testDisplayReadWrite();
  void testEmptyNode();
  void testInterfaceBase();
  void testInterfaceNode();
  void testInterfaceNode64();
  void testMockDisplay();
  void testMockInterface();
  void testMockSerial();
  void testSerialNode();
  void testSubnode();

private:
  static constexpr size_t BUFFER_SIZE{1024};

  uv_loop_t *m_loop{nullptr};
  Interrupt *m_listener{nullptr};
  Interface *m_appInterface{nullptr};
  Interface *m_testInterface{nullptr};
  TestApplication *m_application{nullptr};

  std::thread *m_appThread{nullptr};
  std::thread *m_loopThread{nullptr};
};

void InterfaceNodeTest::setUp()
{
  m_loop = uv_default_loop();
  CPPUNIT_ASSERT(m_loop != nullptr);

  m_listener = TestApplication::makeSignalListener(SIGUSR1, onSignalReceived, m_loop);
  CPPUNIT_ASSERT(m_listener != nullptr);
  m_appInterface = TestApplication::makeUdpInterface("127.0.0.1", 8000, 8001);
  CPPUNIT_ASSERT(m_appInterface != nullptr);
  m_testInterface = TestApplication::makeUdpInterface("127.0.0.1", 8001, 8000);
  CPPUNIT_ASSERT(m_testInterface != nullptr);

  m_application = new TestInterfaceNodeApplication(m_appInterface, m_testInterface);

  m_application->makeDataNode("/testA.bin", 16, 'A');
  m_application->makeDataNode("/testB.bin", 256, 'z');

  m_loopThread = new std::thread{TestApplication::runEventLoop, m_loop};
  m_appThread = new std::thread{TestApplication::runShell, m_application};

  m_application->waitShellResponse();
}

void InterfaceNodeTest::tearDown()
{
  m_application->sendShellCommand("exit");

  m_appThread->join();
  delete m_appThread;

  m_loopThread->join();
  delete m_loopThread;
}

void InterfaceNodeTest::testAccess()
{
  const auto interface = static_cast<Interface *>(init(MockInterface, nullptr));
  CPPUNIT_ASSERT(interface != nullptr);

  VfsNode * const node = new InterfaceNode<
          ParamDesc<IfParameter, IF_RX_AVAILABLE, size_t>,
          ParamDesc<IfParameter, IF_TX_AVAILABLE, size_t>
      >{interface};
  CPPUNIT_ASSERT(node != nullptr);

  // Open subnode before access change

  FsNode *subnode = static_cast<FsNode *>(node->head());
  CPPUNIT_ASSERT(subnode != nullptr);

  Result res;
  FsAccess access;
  size_t count;

  // Read access attribute

  count = std::numeric_limits<size_t>::max();
  res = node->read(FS_NODE_ACCESS, 0, &access, sizeof(access), &count);
  CPPUNIT_ASSERT(res == E_OK);
  CPPUNIT_ASSERT(access == (FS_ACCESS_READ | FS_ACCESS_WRITE));
  CPPUNIT_ASSERT(count == sizeof(access));

  // Write access attribute

  access = 0;
  count = std::numeric_limits<size_t>::max();
  res = node->write(FS_NODE_ACCESS, 0, &access, sizeof(access), &count);
  CPPUNIT_ASSERT(res == E_OK);
  CPPUNIT_ASSERT(count == sizeof(access));

  uint8_t buffer[BUFFER_SIZE] = {0};

  // Try to read data

  res = node->read(FS_NODE_DATA, 0, buffer, sizeof(buffer), nullptr);
  CPPUNIT_ASSERT(res == E_ACCESS);

  // Try to write data

  res = node->write(FS_NODE_DATA, 0, buffer, sizeof(buffer), nullptr);
  CPPUNIT_ASSERT(res == E_ACCESS);

  // Try to fetch next node

  res = fsNodeNext(subnode);
  CPPUNIT_ASSERT(res == E_ENTRY);

  // Try to get head node

  fsNodeFree(subnode);
  subnode = static_cast<FsNode *>(node->head());
  CPPUNIT_ASSERT(subnode == nullptr);

  delete node;
  deinit(interface);
}

void InterfaceNodeTest::testDisplayNode()
{
  const auto interface = static_cast<Interface *>(init(MockDisplay, nullptr));
  CPPUNIT_ASSERT(interface != nullptr);

  VfsNode * const node = new InterfaceNode<
          ParamDesc<DisplayParameter, IF_DISPLAY_ORIENTATION, DisplayOrientation>,
          ParamDesc<DisplayParameter, IF_DISPLAY_RESOLUTION, DisplayResolution, uint16_t, uint16_t>,
          ParamDesc<DisplayParameter, IF_DISPLAY_WINDOW, DisplayWindow, uint16_t, uint16_t, uint16_t, uint16_t>,
          ParamDesc<DisplayParameter, IF_DISPLAY_UPDATE, int>
      >{interface, MockTimeProvider::instance().getTime()};
  CPPUNIT_ASSERT(node != nullptr);

  m_application->injectNode(node, "/display");

  std::vector<std::string> response;
  bool result;

  m_application->sendShellCommand("ls /");
  response = m_application->waitShellResponse();
  result = TestApplication::responseContainsText(response, "display");
  CPPUNIT_ASSERT(result == true);

  // List subnodes

  m_application->sendShellCommand("ls /display");
  response = m_application->waitShellResponse();
  result = TestApplication::responseContainsText(response, "orientation");
  CPPUNIT_ASSERT(result == true);
  result = TestApplication::responseContainsText(response, "resolution");
  CPPUNIT_ASSERT(result == true);
  result = TestApplication::responseContainsText(response, "window");
  CPPUNIT_ASSERT(result == true);
  result = TestApplication::responseContainsText(response, "undefined");
  CPPUNIT_ASSERT(result == true);

  // Test orientation subnode

  m_application->sendShellCommand("echo 3 > /display/orientation");
  m_application->waitShellResponse();

  m_application->sendShellCommand("cat /display/orientation");
  response = m_application->waitShellResponse();
  result = TestApplication::responseContainsText(response, "3");
  CPPUNIT_ASSERT(result == true);

  // Test resolution subnode

  m_application->sendShellCommand("echo \"240 320\" > /display/resolution");
  m_application->waitShellResponse();

  m_application->sendShellCommand("cat /display/resolution");
  response = m_application->waitShellResponse();
  result = TestApplication::responseContainsText(response, "240 320");
  CPPUNIT_ASSERT(result == true);

  // Test window subnode

  m_application->sendShellCommand("echo \"0 0 239 319\" > /display/window");
  m_application->waitShellResponse();

  m_application->sendShellCommand("cat /display/window");
  response = m_application->waitShellResponse();
  result = TestApplication::responseContainsText(response, "0 0 239 319");
  CPPUNIT_ASSERT(result == true);

  // Write and read parent node

  m_application->sendShellCommand("echo \"test pattern\" > /display");
  m_application->waitShellResponse();

  m_application->sendShellCommand("cat /display");
  response = m_application->waitShellResponse();
  result = TestApplication::responseContainsText(response, "test pattern");
  CPPUNIT_ASSERT(result == true);

  // Remove parent node

  m_application->sendShellCommand("rm -r /display");
  m_application->waitShellResponse();

  deinit(interface);
}

void InterfaceNodeTest::testDisplayReadWrite()
{
  const auto interface = static_cast<Interface *>(init(MockDisplay, nullptr));
  CPPUNIT_ASSERT(interface != nullptr);

  VfsNode * const node = new InterfaceNode<>{interface};
  CPPUNIT_ASSERT(node != nullptr);

  uint8_t buffer[BUFFER_SIZE] = {0};
  size_t count;
  uint32_t size;
  Result res;

  res = ifGetParam(interface, IF_SIZE, &size);
  CPPUNIT_ASSERT(res == E_OK);

  // Read overflow

  res = node->read(FS_NODE_DATA, size + 1, buffer, sizeof(buffer), nullptr);
  CPPUNIT_ASSERT(res == E_ADDRESS);

  // Write overflow

  res = node->write(FS_NODE_DATA, size + 1, buffer, sizeof(buffer), nullptr);
  CPPUNIT_ASSERT(res == E_ADDRESS);

  // Read empty buffer

  count = ifRead(interface, buffer, 0);
  CPPUNIT_ASSERT(count == 0);

  // Write empty buffer

  count = ifWrite(interface, buffer, 0);
  CPPUNIT_ASSERT(count == 0);

  delete(node);
  deinit(interface);
}

void InterfaceNodeTest::testEmptyNode()
{
  const auto interface = static_cast<Interface *>(init(MockInterface, nullptr));
  CPPUNIT_ASSERT(interface != nullptr);

  VfsNode * const node = new InterfaceNode<>{interface};
  CPPUNIT_ASSERT(node != nullptr);

  m_application->injectNode(node, "/interface");

  // Try to list subnodes

  m_application->sendShellCommand("ls /interface");
  const auto response = m_application->waitShellResponse();
  const auto result = TestApplication::responseContainsText(response, "node has no descendants");
  CPPUNIT_ASSERT(result == true);

  // Remove parent node

  m_application->sendShellCommand("rm -r /interface");
  m_application->waitShellResponse();

  deinit(interface);
}

void InterfaceNodeTest::testInterfaceBase()
{
  const auto interface = static_cast<Interface *>(init(MockInterface, nullptr));
  CPPUNIT_ASSERT(interface != nullptr);

  VfsNode * const node = new InterfaceNode<>{interface};
  CPPUNIT_ASSERT(node != nullptr);

  Interface *base;
  Result res;
  size_t count;

  count = std::numeric_limits<size_t>::max();
  res = node->read(static_cast<FsFieldType>(VfsNode::VFS_NODE_INTERFACE), 0, &base, sizeof(base), &count);
  CPPUNIT_ASSERT(res == E_OK);
  CPPUNIT_ASSERT(count == sizeof(base));

  res = node->read(static_cast<FsFieldType>(VfsNode::VFS_NODE_INTERFACE), 1, &base, sizeof(base), nullptr);
  CPPUNIT_ASSERT(res == E_VALUE);

  res = node->read(static_cast<FsFieldType>(VfsNode::VFS_NODE_INTERFACE), 0, &base, 0, nullptr);
  CPPUNIT_ASSERT(res == E_VALUE);

  delete node;
  deinit(interface);
}

void InterfaceNodeTest::testInterfaceNode()
{
  static const std::array<const char *, 10> SUBNODE_NAMES = {{
      "rx_available",
      "rx_pending",
      "rx_watermark",
      "tx_available",
      "tx_pending",
      "tx_watermark",
      "address",
      "rate",
      "position",
      "size"
  }};

  const auto interface = static_cast<Interface *>(init(MockInterface, nullptr));
  CPPUNIT_ASSERT(interface != nullptr);

  VfsNode * const node = new InterfaceNode<
          ParamDesc<IfParameter, IF_RX_AVAILABLE, size_t>,
          ParamDesc<IfParameter, IF_RX_PENDING, size_t>,
          ParamDesc<IfParameter, IF_RX_WATERMARK, size_t>,
          ParamDesc<IfParameter, IF_TX_AVAILABLE, size_t>,
          ParamDesc<IfParameter, IF_TX_PENDING, size_t>,
          ParamDesc<IfParameter, IF_TX_WATERMARK, size_t>,
          ParamDesc<IfParameter, IF_ADDRESS, uint32_t>,
          ParamDesc<IfParameter, IF_RATE, uint32_t>,
          ParamDesc<IfParameter, IF_POSITION, uint32_t>,
          ParamDesc<IfParameter, IF_SIZE, uint32_t>,
          ParamDesc<IfParameter, IF_STATUS, int>
      >{interface, MockTimeProvider::instance().getTime()};
  CPPUNIT_ASSERT(node != nullptr);

  m_application->injectNode(node, "/interface");

  std::vector<std::string> response;
  bool result;

  m_application->sendShellCommand("ls /");
  response = m_application->waitShellResponse();
  result = TestApplication::responseContainsText(response, "interface");
  CPPUNIT_ASSERT(result == true);

  // List subnodes

  m_application->sendShellCommand("ls /interface");
  response = m_application->waitShellResponse();
  result = TestApplication::responseContainsText(response, "undefined");
  CPPUNIT_ASSERT(result == true);

  for (size_t i = 0; i < SUBNODE_NAMES.size(); ++i)
  {
    result = TestApplication::responseContainsText(response, SUBNODE_NAMES[i]);
    CPPUNIT_ASSERT(result == true);
  }

  // Write and read subnodes

  for (size_t i = 0; i < SUBNODE_NAMES.size(); ++i)
  {
    const std::string command0 = std::string{"echo "} + std::to_string(i) + " > /interface/" + SUBNODE_NAMES[i];
    const std::string command1 = std::string{"cat /interface/"} + SUBNODE_NAMES[i];

    m_application->sendShellCommand(command0.data());
    m_application->waitShellResponse();

    m_application->sendShellCommand(command1.data());
    response = m_application->waitShellResponse();
    result = TestApplication::responseContainsText(response, std::to_string(i));
    CPPUNIT_ASSERT(result == true);
  }

  // Write and read parent node

  m_application->sendShellCommand("echo \"test pattern\" > /interface");
  m_application->waitShellResponse();

  m_application->sendShellCommand("cat /interface");
  response = m_application->waitShellResponse();
  result = TestApplication::responseContainsText(response, "test pattern");
  CPPUNIT_ASSERT(result == true);

  // Remove parent node

  m_application->sendShellCommand("rm -r /interface");
  m_application->waitShellResponse();

  m_application->sendShellCommand("ls /");
  response = m_application->waitShellResponse();
  result = TestApplication::responseContainsText(response, "interface");
  CPPUNIT_ASSERT(result == false);

  deinit(interface);
}

void InterfaceNodeTest::testInterfaceNode64()
{
  static const std::array<const char *, 3> SUBNODE_NAMES = {{
      "address",
      "position",
      "size"
  }};

  const auto interface = static_cast<Interface *>(init(MockInterface, nullptr));
  CPPUNIT_ASSERT(interface != nullptr);

  VfsNode * const node = new InterfaceNode<
          ParamDesc<IfParameter, IF_ADDRESS_64, uint64_t>,
          ParamDesc<IfParameter, IF_POSITION_64, uint64_t>,
          ParamDesc<IfParameter, IF_SIZE_64, uint64_t>
      >{interface, MockTimeProvider::instance().getTime()};
  CPPUNIT_ASSERT(node != nullptr);

  m_application->injectNode(node, "/interface");

  std::vector<std::string> response;
  bool result;

  // List subnodes

  m_application->sendShellCommand("ls /interface");
  response = m_application->waitShellResponse();

  for (size_t i = 0; i < SUBNODE_NAMES.size(); ++i)
  {
    result = TestApplication::responseContainsText(response, SUBNODE_NAMES[i]);
    CPPUNIT_ASSERT(result == true);
  }

  // Write and read subnodes

  for (size_t i = 0; i < SUBNODE_NAMES.size(); ++i)
  {
    const std::string command0 = std::string{"echo "} + std::to_string(i) + " > /interface/" + SUBNODE_NAMES[i];
    const std::string command1 = std::string{"cat /interface/"} + SUBNODE_NAMES[i];

    m_application->sendShellCommand(command0.data());
    m_application->waitShellResponse();

    m_application->sendShellCommand(command1.data());
    response = m_application->waitShellResponse();
    result = TestApplication::responseContainsText(response, std::to_string(i));
    CPPUNIT_ASSERT(result == true);
  }

  // Remove parent node

  m_application->sendShellCommand("rm -r /interface");
  m_application->waitShellResponse();

  deinit(interface);
}

void InterfaceNodeTest::testMockDisplay()
{
  Result res;

  const auto interface = static_cast<Interface *>(init(MockDisplay, nullptr));
  CPPUNIT_ASSERT(interface != nullptr);

  res = ifGetParam(interface, IF_STATUS, nullptr);
  CPPUNIT_ASSERT(res == E_INVALID);

  res = ifSetParam(interface, IF_STATUS, nullptr);
  CPPUNIT_ASSERT(res == E_INVALID);

  deinit(interface);
}

void InterfaceNodeTest::testMockInterface()
{
  Result res;

  const auto interface = static_cast<Interface *>(init(MockInterface, nullptr));
  CPPUNIT_ASSERT(interface != nullptr);

  res = ifGetParam(interface, IF_STATUS, nullptr);
  CPPUNIT_ASSERT(res == E_INVALID);

  res = ifSetParam(interface, IF_STATUS, nullptr);
  CPPUNIT_ASSERT(res == E_INVALID);

  deinit(interface);
}

void InterfaceNodeTest::testMockSerial()
{
  Result res;

  const auto interface = static_cast<Interface *>(init(MockSerial, nullptr));
  CPPUNIT_ASSERT(interface != nullptr);

  res = ifGetParam(interface, IF_STATUS, nullptr);
  CPPUNIT_ASSERT(res == E_INVALID);

  res = ifSetParam(interface, IF_STATUS, nullptr);
  CPPUNIT_ASSERT(res == E_INVALID);

  deinit(interface);
}

void InterfaceNodeTest::testSerialNode()
{
  static const std::array<const char *, 4> SUBNODE_NAMES = {{
      "cts",
      "rts",
      "dsr",
      "dtr"
  }};

  const auto interface = static_cast<Interface *>(init(MockSerial, nullptr));
  CPPUNIT_ASSERT(interface != nullptr);

  VfsNode * const node = new InterfaceNode<
          ParamDesc<SerialParameter, IF_SERIAL_PARITY, SerialParity>,
          ParamDesc<SerialParameter, IF_SERIAL_CTS, bool>,
          ParamDesc<SerialParameter, IF_SERIAL_RTS, bool>,
          ParamDesc<SerialParameter, IF_SERIAL_DSR, bool>,
          ParamDesc<SerialParameter, IF_SERIAL_DTR, bool>
      >{interface, MockTimeProvider::instance().getTime()};
  CPPUNIT_ASSERT(node != nullptr);

  m_application->injectNode(node, "/serial");

  std::vector<std::string> response;
  bool result;

  m_application->sendShellCommand("ls /");
  response = m_application->waitShellResponse();
  result = TestApplication::responseContainsText(response, "serial");
  CPPUNIT_ASSERT(result == true);

  // List subnodes

  m_application->sendShellCommand("ls /serial");
  response = m_application->waitShellResponse();
  result = TestApplication::responseContainsText(response, "parity");
  CPPUNIT_ASSERT(result == true);
  result = TestApplication::responseContainsText(response, "cts");
  CPPUNIT_ASSERT(result == true);
  result = TestApplication::responseContainsText(response, "rts");
  CPPUNIT_ASSERT(result == true);
  result = TestApplication::responseContainsText(response, "dsr");
  CPPUNIT_ASSERT(result == true);
  result = TestApplication::responseContainsText(response, "dtr");
  CPPUNIT_ASSERT(result == true);

  // Test parity subnode

  m_application->sendShellCommand("echo 1 > /serial/parity");
  m_application->waitShellResponse();

  m_application->sendShellCommand("cat /serial/parity");
  response = m_application->waitShellResponse();
  result = TestApplication::responseContainsText(response, "1");
  CPPUNIT_ASSERT(result == true);

  // Write and read subnodes

  for (size_t i = 0; i < SUBNODE_NAMES.size(); ++i)
  {
    const std::string command0 = std::string{"echo 1 > /serial/"} + SUBNODE_NAMES[i];
    const std::string command1 = std::string{"cat /serial/"} + SUBNODE_NAMES[i];

    m_application->sendShellCommand(command0.data());
    m_application->waitShellResponse();

    m_application->sendShellCommand(command1.data());
    response = m_application->waitShellResponse();
    result = TestApplication::responseContainsText(response, "1");
    CPPUNIT_ASSERT(result == true);
  }

  // Write and read parent node

  m_application->sendShellCommand("echo \"test pattern\" > /serial");
  m_application->waitShellResponse();

  m_application->sendShellCommand("cat /serial");
  response = m_application->waitShellResponse();
  result = TestApplication::responseContainsText(response, "test pattern");
  CPPUNIT_ASSERT(result == true);

  // Remove parent node

  m_application->sendShellCommand("rm -r /serial");
  m_application->waitShellResponse();

  deinit(interface);
}

void InterfaceNodeTest::testSubnode()
{
  const auto interface = static_cast<Interface *>(init(MockInterface, nullptr));
  CPPUNIT_ASSERT(interface != nullptr);

  VfsNode * const node = new InterfaceNode<
          ParamDesc<IfParameter, IF_RX_AVAILABLE, size_t>,
          ParamDesc<IfParameter, IF_TX_AVAILABLE, size_t>
      >{interface};
  CPPUNIT_ASSERT(node != nullptr);

  FsNode *subnode = static_cast<FsNode *>(node->head());
  CPPUNIT_ASSERT(subnode != nullptr);

  uint8_t buffer[BUFFER_SIZE] = {0};
  Result res;
  FsLength length;
  FsAccess access;
  size_t count;

  // Try to read to a buffer of insufficient size

  count = std::numeric_limits<size_t>::max();
  res = fsNodeRead(subnode, FS_NODE_DATA, 0, &buffer, 0, &count);
  CPPUNIT_ASSERT(res == E_VALUE);
  CPPUNIT_ASSERT(count == std::numeric_limits<size_t>::max());

  // Serialization overflow

  const size_t value = std::numeric_limits<size_t>::max();
  ifSetParam(interface, IF_RX_AVAILABLE, &value);

  count = std::numeric_limits<size_t>::max();
  res = fsNodeRead(subnode, FS_NODE_DATA, 0, &buffer,
      TerminalHelpers::serializedValueLength<decltype(value)>(), &count);
  CPPUNIT_ASSERT(res == E_VALUE);
  CPPUNIT_ASSERT(count == std::numeric_limits<size_t>::max());

  // Write empty buffer

  count = std::numeric_limits<size_t>::max();
  res = fsNodeWrite(subnode, FS_NODE_DATA, 0, &buffer, 0, &count);
  CPPUNIT_ASSERT(res == E_OK);
  CPPUNIT_ASSERT(count == 0);

  // Try to read unaligned data

  res = fsNodeRead(subnode, FS_NODE_DATA, 1, &buffer, sizeof(buffer), nullptr);
  CPPUNIT_ASSERT(res == E_EMPTY);

  // Try to write unaligned data

  res = fsNodeWrite(subnode, FS_NODE_DATA, 1, &buffer, sizeof(buffer), nullptr);
  CPPUNIT_ASSERT(res == E_FULL);

  // Get attribute length

  length = std::numeric_limits<FsLength>::max();
  res = fsNodeLength(subnode, FS_NODE_ACCESS, &length);
  CPPUNIT_ASSERT(res == E_OK);
  CPPUNIT_ASSERT(length == sizeof(access));

  // Read access attribute

  count = std::numeric_limits<size_t>::max();
  res = fsNodeRead(subnode, FS_NODE_ACCESS, 0, &access, sizeof(access), &count);
  CPPUNIT_ASSERT(res == E_OK);
  CPPUNIT_ASSERT(access == (FS_ACCESS_READ | FS_ACCESS_WRITE));
  CPPUNIT_ASSERT(count == sizeof(access));

  // Change access

  access = 0;
  count = std::numeric_limits<size_t>::max();
  res = fsNodeWrite(subnode, FS_NODE_ACCESS, 0, &access, sizeof(access), &count);
  CPPUNIT_ASSERT(res == E_OK);
  CPPUNIT_ASSERT(count == sizeof(access));

  // Try to read data

  res = fsNodeRead(subnode, FS_NODE_DATA, 0, &buffer, sizeof(buffer), nullptr);
  CPPUNIT_ASSERT(res == E_ACCESS);

  // Try to write data

  res = fsNodeWrite(subnode, FS_NODE_DATA, 0, &buffer, sizeof(buffer), nullptr);
  CPPUNIT_ASSERT(res == E_ACCESS);

  fsNodeFree(subnode);
  delete node;
  deinit(interface);
}

CPPUNIT_TEST_SUITE_REGISTRATION(InterfaceNodeTest);

int main(int, char *[])
{
  CPPUNIT_NS::Test * const suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();

  CppUnit::TextUi::TestRunner runner;
  runner.addTest(suite);

  const bool sucessful = runner.run();
  return sucessful ? EXIT_SUCCESS : EXIT_FAILURE;
}
