/*
 * Main.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Shell/ShellHelpers.hpp"
#include "Vfs/VfsDataNode.hpp"
#include "Vfs/VfsDirectory.hpp"
#include "Vfs/VfsHandle.hpp"
#include "Vfs/VfsMountpoint.hpp"
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cstring>

class VfsTest: public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(VfsTest);
  CPPUNIT_TEST(testDataNode);
  CPPUNIT_TEST(testDataNodeAccess);
  CPPUNIT_TEST(testDataNodeLength);
  CPPUNIT_TEST(testDataNodeRead);
  CPPUNIT_TEST(testDataNodeReserve);
  CPPUNIT_TEST(testDataNodeWrite);
  CPPUNIT_TEST(testHandle);
  CPPUNIT_TEST(testNodeCreationFailures);
  CPPUNIT_TEST(testNodeDeletionFailures);
  CPPUNIT_TEST(testNodeInjection);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testDataNode();
  void testDataNodeAccess();
  void testDataNodeLength();
  void testDataNodeRead();
  void testDataNodeReserve();
  void testDataNodeWrite();
  void testHandle();
  void testNodeCreationFailures();
  void testNodeDeletionFailures();
  void testNodeInjection();

private:
  static constexpr size_t BUFFER_SIZE{1024};

  FsHandle *handle{nullptr};
};

void VfsTest::setUp()
{
  handle = static_cast<FsHandle *>(init(VfsHandleClass, nullptr));
  CPPUNIT_ASSERT(handle != nullptr);
}

void VfsTest::tearDown()
{
  deinit(handle);
}

void VfsTest::testDataNode()
{
  static const std::array<FsFieldDescriptor, 2> desc = {{
      {
          "test",
          strlen("test") + 1,
          FS_NODE_NAME
      }, {
          0,
          0,
          FS_NODE_DATA
      }
  }};

  VfsDataNode * const node = new VfsDataNode{};
  CPPUNIT_ASSERT(node != nullptr);

  VfsNode *child;
  Result res;

  // Try to create a descendant node
  res = node->create(desc.data(), desc.size());
  CPPUNIT_ASSERT(res == E_INVALID);

  // Try to fetch child node
  child = node->fetch(nullptr);
  CPPUNIT_ASSERT(child == nullptr);

  // Try to remove some node
  res = node->remove(nullptr);
  CPPUNIT_ASSERT(res == E_INVALID);

  delete node;
}

void VfsTest::testDataNodeAccess()
{
  VfsDataNode * const node = new VfsDataNode{};
  CPPUNIT_ASSERT(node != nullptr);

  uint8_t buffer[BUFFER_SIZE] = {0};
  Result res;
  FsAccess access;

  // Data read access error

  access = FS_ACCESS_WRITE;
  res = node->write(FS_NODE_ACCESS, 0, &access, sizeof(access), nullptr);
  CPPUNIT_ASSERT(res == E_OK);
  res = node->read(FS_NODE_DATA, 0, &buffer, sizeof(buffer), nullptr);
  CPPUNIT_ASSERT(res == E_ACCESS);

  // Data write access error

  access = FS_ACCESS_READ;
  res = node->write(FS_NODE_ACCESS, 0, &access, sizeof(access), nullptr);
  CPPUNIT_ASSERT(res == E_OK);
  res = node->write(FS_NODE_DATA, 0, &buffer, sizeof(buffer), nullptr);
  CPPUNIT_ASSERT(res == E_ACCESS);

  delete node;
}

void VfsTest::testDataNodeLength()
{
  static const char NODE_NAME[] = "node.txt";

  VfsDataNode * const node = new VfsDataNode{};
  CPPUNIT_ASSERT(node != nullptr);

  const auto ok = node->rename(NODE_NAME);
  CPPUNIT_ASSERT(ok == true);

  FsLength length;
  Result res;

  length = std::numeric_limits<FsLength>::max();
  res = node->length(FS_NODE_ACCESS, &length);
  CPPUNIT_ASSERT(res == E_OK);
  CPPUNIT_ASSERT(length == sizeof(FsAccess));

  length = std::numeric_limits<FsLength>::max();
  res = node->length(FS_NODE_ID, &length);
  CPPUNIT_ASSERT(res == E_OK);
  CPPUNIT_ASSERT(length == sizeof(FsIdentifier));

  length = std::numeric_limits<FsLength>::max();
  res = node->length(FS_NODE_NAME, &length);
  CPPUNIT_ASSERT(res == E_OK);
  CPPUNIT_ASSERT(length == strlen(NODE_NAME) + 1);

  length = std::numeric_limits<FsLength>::max();
  res = node->length(FS_NODE_TIME, &length);
  CPPUNIT_ASSERT(res == E_OK);
  CPPUNIT_ASSERT(length == sizeof(time64_t));

  length = std::numeric_limits<FsLength>::max();
  res = node->length(FS_NODE_DEVICE, &length);
  CPPUNIT_ASSERT(res == E_INVALID);
  CPPUNIT_ASSERT(length == std::numeric_limits<FsLength>::max());

  delete node;
}

void VfsTest::testDataNodeRead()
{
  static const char NODE_NAME[] = "node.txt";

  VfsDataNode * const node = new VfsDataNode{};
  CPPUNIT_ASSERT(node != nullptr);

  const auto ok = node->rename(NODE_NAME);
  CPPUNIT_ASSERT(ok == true);

  size_t length;
  Result res;

  // Read access attribute

  FsAccess bufferAccess;

  length = std::numeric_limits<size_t>::max();
  res = node->read(FS_NODE_ACCESS, 0, &bufferAccess, sizeof(bufferAccess), &length);
  CPPUNIT_ASSERT(res == E_OK);
  CPPUNIT_ASSERT(length == sizeof(bufferAccess));
  CPPUNIT_ASSERT(bufferAccess == (FS_ACCESS_READ | FS_ACCESS_WRITE));

  length = std::numeric_limits<size_t>::max();
  res = node->read(FS_NODE_ACCESS, 1, &bufferAccess, sizeof(bufferAccess), &length);
  CPPUNIT_ASSERT(res == E_VALUE);
  CPPUNIT_ASSERT(length == std::numeric_limits<size_t>::max());

  res = node->read(FS_NODE_ACCESS, 0, &bufferAccess, 0, nullptr);
  CPPUNIT_ASSERT(res == E_VALUE);

  // Read identifier attribute

  FsIdentifier bufferIdentifier;

  length = std::numeric_limits<size_t>::max();
  res = node->read(FS_NODE_ID, 0, &bufferIdentifier, sizeof(bufferIdentifier), &length);
  CPPUNIT_ASSERT(res == E_OK);
  CPPUNIT_ASSERT(length == sizeof(bufferIdentifier));
  CPPUNIT_ASSERT(bufferIdentifier == reinterpret_cast<FsIdentifier>(node));

  res = node->read(FS_NODE_ID, 1, &bufferIdentifier, sizeof(bufferIdentifier), nullptr);
  CPPUNIT_ASSERT(res == E_VALUE);

  res = node->read(FS_NODE_ID, 0, &bufferIdentifier, 0, nullptr);
  CPPUNIT_ASSERT(res == E_VALUE);

  // Read time

  time64_t bufferTime;

  length = std::numeric_limits<size_t>::max();
  res = node->read(FS_NODE_TIME, 0, &bufferTime, sizeof(bufferTime), &length);
  CPPUNIT_ASSERT(res == E_OK);
  CPPUNIT_ASSERT(length == sizeof(bufferTime));
  CPPUNIT_ASSERT(bufferTime == 0);

  res = node->read(FS_NODE_TIME, 1, &bufferTime, sizeof(bufferTime), nullptr);
  CPPUNIT_ASSERT(res == E_VALUE);

  res = node->read(FS_NODE_TIME, 0, &bufferTime, 0, nullptr);
  CPPUNIT_ASSERT(res == E_VALUE);

  // Read name

  char bufferName[BUFFER_SIZE];

  length = std::numeric_limits<size_t>::max();
  res = node->read(FS_NODE_NAME, 0, &bufferName, sizeof(bufferName), &length);
  CPPUNIT_ASSERT(res == E_OK);
  CPPUNIT_ASSERT(length == strlen(NODE_NAME) + 1);
  CPPUNIT_ASSERT(strcmp(bufferName, NODE_NAME) == 0);

  res = node->read(FS_NODE_NAME, 1, &bufferName, sizeof(bufferName), nullptr);
  CPPUNIT_ASSERT(res == E_VALUE);

  res = node->read(FS_NODE_NAME, 0, &bufferName, 0, nullptr);
  CPPUNIT_ASSERT(res == E_VALUE);

  // Data read overflow

  uint8_t bufferData[BUFFER_SIZE];

  res = node->read(FS_NODE_DATA, sizeof(bufferData), &bufferData, sizeof(bufferData), nullptr);
  CPPUNIT_ASSERT(res == E_VALUE);

  // Read incorrect attribute

  FsDevice bufferDevice;

  length = std::numeric_limits<size_t>::max();
  res = node->read(FS_NODE_DEVICE, 0, &bufferDevice, sizeof(bufferDevice), &length);
  CPPUNIT_ASSERT(res == E_INVALID);
  CPPUNIT_ASSERT(length == std::numeric_limits<size_t>::max());

  delete node;
}

void VfsTest::testDataNodeReserve()
{
  VfsDataNode * const node = new VfsDataNode{};
  CPPUNIT_ASSERT(node != nullptr);

  bool ok;

  ok = node->reserve(1000, '\0');
  CPPUNIT_ASSERT(ok == true);
  ok = node->reserve(0, '\0');
  CPPUNIT_ASSERT(ok == true);

  ok = node->reserve("test pattern", 12);
  CPPUNIT_ASSERT(ok == true);
  ok = node->reserve(nullptr, 0);
  CPPUNIT_ASSERT(ok == true);

  ok = node->reserve("test pattern");
  CPPUNIT_ASSERT(ok == true);
  ok = node->reserve(nullptr);
  CPPUNIT_ASSERT(ok == true);

  ok = node->rename("test");
  CPPUNIT_ASSERT(ok == true);
  ok = node->rename(nullptr);
  CPPUNIT_ASSERT(ok == true);

  delete node;
}

void VfsTest::testDataNodeWrite()
{
  VfsDataNode * const node = new VfsDataNode{};
  CPPUNIT_ASSERT(node != nullptr);

  size_t length;
  Result res;

  // Write access attribute

  const FsAccess bufferAccess = FS_ACCESS_READ;
  FsAccess bufferAccessCheck = 0;

  res = node->write(FS_NODE_ACCESS, 1, &bufferAccess, sizeof(bufferAccess), &length);
  CPPUNIT_ASSERT(res == E_VALUE);

  res = node->write(FS_NODE_ACCESS, 0, &bufferAccess, 1, &length);
  CPPUNIT_ASSERT(res == E_VALUE);

  res = node->write(FS_NODE_ACCESS, 0, &bufferAccess, sizeof(bufferAccess), &length);
  CPPUNIT_ASSERT(res == E_OK);
  CPPUNIT_ASSERT(length == sizeof(bufferAccess));

  res = node->read(FS_NODE_ACCESS, 0, &bufferAccessCheck, sizeof(bufferAccessCheck), &length);
  CPPUNIT_ASSERT(res == E_OK);
  CPPUNIT_ASSERT(length == sizeof(bufferAccessCheck));
  CPPUNIT_ASSERT(bufferAccessCheck == bufferAccess);

  // Write access attribute

  const time64_t bufferTime = 315964800;
  time64_t bufferTimeCheck = 0;

  res = node->write(FS_NODE_TIME, 1, &bufferTime, sizeof(bufferTime), &length);
  CPPUNIT_ASSERT(res == E_VALUE);

  res = node->write(FS_NODE_TIME, 0, &bufferTime, 1, &length);
  CPPUNIT_ASSERT(res == E_VALUE);

  res = node->write(FS_NODE_TIME, 0, &bufferTime, sizeof(bufferTime), &length);
  CPPUNIT_ASSERT(res == E_OK);
  CPPUNIT_ASSERT(length == sizeof(bufferTime));

  res = node->read(FS_NODE_TIME, 0, &bufferTimeCheck, sizeof(bufferTimeCheck), &length);
  CPPUNIT_ASSERT(res == E_OK);
  CPPUNIT_ASSERT(length == sizeof(bufferTimeCheck));
  CPPUNIT_ASSERT(bufferTimeCheck == bufferTime);

  // Write incorrect attribute

  const FsDevice bufferDevice = 0;

  res = node->write(FS_NODE_DEVICE, 0, &bufferDevice, sizeof(bufferDevice), &length);
  CPPUNIT_ASSERT(res == E_INVALID);

  delete node;
}

void VfsTest::testHandle()
{
  const Result res = fsHandleSync(handle);
  CPPUNIT_ASSERT(res == E_OK);
}

void VfsTest::testNodeCreationFailures()
{
  const FsAccess access = FS_ACCESS_READ | FS_ACCESS_WRITE;
  const time64_t timestamp = 0;
  Result res;

  auto dir = new VfsDirectory{};
  CPPUNIT_ASSERT(dir != nullptr);
  auto node = new VfsDataNode{};
  CPPUNIT_ASSERT(node != nullptr);

  // Try to create node with incorrect access size

  const std::array<FsFieldDescriptor, 1> accessSizeTest = {{
      {
          &access,
          0,
          FS_NODE_ACCESS
      }
  }};
  res = dir->create(accessSizeTest.data(), accessSizeTest.size());
  CPPUNIT_ASSERT(res == E_VALUE);

  // Try to create node with incorrect timestamp size

  const std::array<FsFieldDescriptor, 1> timeSizeTest = {{
      {
          &timestamp,
          0,
          FS_NODE_TIME
      }
  }};
  res = dir->create(timeSizeTest.data(), timeSizeTest.size());
  CPPUNIT_ASSERT(res == E_VALUE);

  // Try to create node with incorrect name length

  const std::array<FsFieldDescriptor, 1> nameSizeTest = {{
      {
          "",
          0,
          FS_NODE_NAME
      }
  }};
  res = dir->create(nameSizeTest.data(), nameSizeTest.size());
  CPPUNIT_ASSERT(res == E_VALUE);

  // Try to create node with incorrect node object size

  const std::array<FsFieldDescriptor, 1> objectSizeTest = {{
      {
          &node,
          0,
          static_cast<FsFieldType>(VfsNode::VFS_NODE_OBJECT)
      }
  }};
  res = dir->create(objectSizeTest.data(), objectSizeTest.size());
  CPPUNIT_ASSERT(res == E_VALUE);

  // Mixed list of descriptors

  const std::array<FsFieldDescriptor, 2> mixedListTest = {{
      {
          "test",
          strlen("test") + 1,
          FS_NODE_NAME
      }, {
          &node,
          sizeof(node),
          static_cast<FsFieldType>(VfsNode::VFS_NODE_OBJECT)
      }
  }};
  res = dir->create(mixedListTest.data(), mixedListTest.size());
  CPPUNIT_ASSERT(res == E_INVALID);

  // Try to create node without name

  const std::array<FsFieldDescriptor, 1> unnamedNodeTest = {{
      {
          &access,
          sizeof(access),
          FS_NODE_ACCESS
      }
  }};
  res = dir->create(unnamedNodeTest.data(), unnamedNodeTest.size());
  CPPUNIT_ASSERT(res == E_VALUE);

  delete node;
  delete dir;
}

void VfsTest::testNodeDeletionFailures()
{
  auto dir = new VfsDirectory{};
  CPPUNIT_ASSERT(dir != nullptr);

  auto proxy = static_cast<FsNode *>(fsHandleRoot(handle));
  const auto res = dir->remove(proxy);
  CPPUNIT_ASSERT(res == E_ENTRY);
  fsNodeFree(proxy);

  delete dir;
}

void VfsTest::testNodeInjection()
{
  Result res;

  auto node = new VfsDataNode{};
  CPPUNIT_ASSERT(node != nullptr);

  // Test null pointer injection
  res = ShellHelpers::injectNode(handle, nullptr, "/test");
  CPPUNIT_ASSERT(res == E_VALUE);

  // Test incorrect node name
  res = ShellHelpers::injectNode(handle, node, "/");
  CPPUNIT_ASSERT(res == E_VALUE);

  // Test incorrect parent name
  res = ShellHelpers::injectNode(handle, node, "/undefined/test");
  CPPUNIT_ASSERT(res == E_ENTRY);

  delete node;
}

CPPUNIT_TEST_SUITE_REGISTRATION(VfsTest);

int main(int, char *[])
{
  CPPUNIT_NS::Test * const suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();

  CppUnit::TextUi::TestRunner runner;
  runner.addTest(suite);

  const bool sucessful = runner.run();
  return sucessful ? EXIT_SUCCESS : EXIT_FAILURE;
}
