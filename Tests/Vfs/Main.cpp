/*
 * Main.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Vfs/VfsDataNode.hpp"
#include "Vfs/VfsDirectory.hpp"
#include "Vfs/VfsHandle.hpp"
#include "Vfs/VfsMountpoint.hpp"
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cstring>

extern "C" void *__libc_malloc(size_t);

static unsigned int mallocHookFails = 0;

extern "C" void *malloc(size_t size)
{
  if (mallocHookFails)
  {
    if (--mallocHookFails)
      return __libc_malloc(size);
    else
      return 0;
  }
  else
    return __libc_malloc(size);
}

class TerminalTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(TerminalTest);
  CPPUNIT_TEST(testDataNode);
  CPPUNIT_TEST(testDataNodeFailures);
  CPPUNIT_TEST(testDataNodeLength);
  CPPUNIT_TEST(testDataNodeRead);
  CPPUNIT_TEST(testDataNodeReserve);
  CPPUNIT_TEST(testDataNodeWrite);
  CPPUNIT_TEST(testHandle);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testDataNode();
  void testDataNodeFailures();
  void testDataNodeLength();
  void testDataNodeRead();
  void testDataNodeReserve();
  void testDataNodeWrite();
  void testHandle();

private:
  static constexpr size_t BUFFER_SIZE{1024};

  FsHandle *handle{nullptr};
};

void TerminalTest::setUp()
{
  handle = static_cast<FsHandle *>(init(VfsHandleClass, nullptr));
  CPPUNIT_ASSERT(handle != nullptr);
}

void TerminalTest::tearDown()
{
  deinit(handle);
}

void TerminalTest::testDataNode()
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

  VfsDataNode * const node = new VfsDataNode{nullptr};
  CPPUNIT_ASSERT(node != nullptr);

  VfsNode *child;
  Result res;

  // Try to create a descendant node
  res = node->create(desc.data(), desc.size());
  CPPUNIT_ASSERT(res != E_OK);

  // Try to fetch child node
  child = node->fetch(nullptr);
  CPPUNIT_ASSERT(child == nullptr);

  // Try to remove some node
  res = node->remove(nullptr);
  CPPUNIT_ASSERT(res == E_INVALID);
}

void TerminalTest::testDataNodeFailures()
{
  static const char NODE_NAME[] = "node.txt";

  VfsDataNode * const node = new VfsDataNode{NODE_NAME};
  CPPUNIT_ASSERT(node != nullptr);

  Result res;
  bool ok;

  mallocHookFails = 1;
  ok = node->reserve(1000, '\0');
  CPPUNIT_ASSERT(ok == false);

  mallocHookFails = 1;
  ok = node->reserve("test pattern", 12);
  CPPUNIT_ASSERT(ok == false);

  const uint8_t bufferData[BUFFER_SIZE]{};

  mallocHookFails = 1;
  res = node->write(FS_NODE_DATA, 0, &bufferData, sizeof(bufferData), nullptr);
  CPPUNIT_ASSERT(res != E_OK);

  delete node;
}

void TerminalTest::testDataNodeLength()
{
  static const char NODE_NAME[] = "node.txt";

  VfsDataNode * const node = new VfsDataNode{NODE_NAME};
  CPPUNIT_ASSERT(node != nullptr);

  FsLength length;
  Result res;

  res = node->length(FS_NODE_ACCESS, &length);
  CPPUNIT_ASSERT(res == E_OK);
  CPPUNIT_ASSERT(length == sizeof(FsAccess));

  res = node->length(FS_NODE_ID, &length);
  CPPUNIT_ASSERT(res == E_OK);
  CPPUNIT_ASSERT(length == sizeof(FsIdentifier));

  res = node->length(FS_NODE_NAME, &length);
  CPPUNIT_ASSERT(res == E_OK);
  CPPUNIT_ASSERT(length == strlen(NODE_NAME) + 1);

  res = node->length(FS_NODE_TIME, &length);
  CPPUNIT_ASSERT(res == E_OK);
  CPPUNIT_ASSERT(length == sizeof(time64_t));

  res = node->length(FS_NODE_DEVICE, &length);
  CPPUNIT_ASSERT(res == E_INVALID);

  delete node;
}

void TerminalTest::testDataNodeRead()
{
  static const char NODE_NAME[] = "node.txt";

  VfsDataNode * const node = new VfsDataNode{NODE_NAME};
  CPPUNIT_ASSERT(node != nullptr);

  size_t length;
  Result res;

  // Read access attribute

  FsAccess bufferAccess;

  res = node->read(FS_NODE_ACCESS, 0, &bufferAccess, sizeof(bufferAccess), &length);
  CPPUNIT_ASSERT(res == E_OK);
  CPPUNIT_ASSERT(length == sizeof(bufferAccess));
  CPPUNIT_ASSERT(bufferAccess == (FS_ACCESS_READ | FS_ACCESS_WRITE));

  res = node->read(FS_NODE_ACCESS, 1, &bufferAccess, sizeof(bufferAccess), &length);
  CPPUNIT_ASSERT(res != E_OK);

  res = node->read(FS_NODE_ACCESS, 0, &bufferAccess, 1, &length);
  CPPUNIT_ASSERT(res != E_OK);

  // Read identifier attribute

  FsIdentifier bufferIdentifier;

  res = node->read(FS_NODE_ID, 0, &bufferIdentifier, sizeof(bufferIdentifier), &length);
  CPPUNIT_ASSERT(res == E_OK);
  CPPUNIT_ASSERT(length == sizeof(bufferIdentifier));
  CPPUNIT_ASSERT(bufferIdentifier == reinterpret_cast<FsIdentifier>(node));

  res = node->read(FS_NODE_ID, 1, &bufferIdentifier, sizeof(bufferIdentifier), &length);
  CPPUNIT_ASSERT(res != E_OK);

  res = node->read(FS_NODE_ID, 0, &bufferIdentifier, 1, &length);
  CPPUNIT_ASSERT(res != E_OK);

  // Read time

  time64_t bufferTime;

  res = node->read(FS_NODE_TIME, 0, &bufferTime, sizeof(bufferTime), &length);
  CPPUNIT_ASSERT(res == E_OK);
  CPPUNIT_ASSERT(length == sizeof(bufferTime));
  CPPUNIT_ASSERT(bufferTime == 0);

  res = node->read(FS_NODE_TIME, 1, &bufferTime, sizeof(bufferTime), &length);
  CPPUNIT_ASSERT(res != E_OK);

  res = node->read(FS_NODE_TIME, 0, &bufferTime, 1, &length);
  CPPUNIT_ASSERT(res != E_OK);

  // Read name

  char bufferName[BUFFER_SIZE];

  res = node->read(FS_NODE_NAME, 0, &bufferName, sizeof(bufferName), &length);
  CPPUNIT_ASSERT(res == E_OK);
  CPPUNIT_ASSERT(length == strlen(NODE_NAME) + 1);
  CPPUNIT_ASSERT(strcmp(bufferName, NODE_NAME) == 0);

  res = node->read(FS_NODE_NAME, 1, &bufferName, sizeof(bufferName), &length);
  CPPUNIT_ASSERT(res != E_OK);

  res = node->read(FS_NODE_NAME, 0, &bufferName, 1, &length);
  CPPUNIT_ASSERT(res != E_OK);

  // Data read overflow

  uint8_t bufferData[BUFFER_SIZE];

  res = node->read(FS_NODE_DATA, sizeof(bufferData), &bufferData, sizeof(bufferName), &length);
  CPPUNIT_ASSERT(res != E_OK);

  // Read incorrect attribute

  FsDevice bufferDevice;

  res = node->read(FS_NODE_DEVICE, 0, &bufferDevice, sizeof(bufferDevice), &length);
  CPPUNIT_ASSERT(res == E_INVALID);

  delete node;
}

void TerminalTest::testDataNodeReserve()
{
  static const char NODE_NAME[] = "node.txt";

  VfsDataNode * const node = new VfsDataNode{NODE_NAME};
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

  delete node;
}

void TerminalTest::testDataNodeWrite()
{
  static const char NODE_NAME[] = "node.txt";

  VfsDataNode * const node = new VfsDataNode{NODE_NAME};
  CPPUNIT_ASSERT(node != nullptr);

  size_t length;
  Result res;

  // Write access attribute

  const FsAccess bufferAccess = FS_ACCESS_READ;
  FsAccess bufferAccessCheck = 0;

  res = node->write(FS_NODE_ACCESS, 1, &bufferAccess, sizeof(bufferAccess), &length);
  CPPUNIT_ASSERT(res != E_OK);

  res = node->write(FS_NODE_ACCESS, 0, &bufferAccess, 1, &length);
  CPPUNIT_ASSERT(res != E_OK);

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
  CPPUNIT_ASSERT(res != E_OK);

  res = node->write(FS_NODE_TIME, 0, &bufferTime, 1, &length);
  CPPUNIT_ASSERT(res != E_OK);

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

void TerminalTest::testHandle()
{
  const Result res = fsHandleSync(handle);
  CPPUNIT_ASSERT(res == E_OK);
}

CPPUNIT_TEST_SUITE_REGISTRATION(TerminalTest);

int main(int, char *[])
{
  CPPUNIT_NS::Test * const suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();

  CppUnit::TextUi::TestRunner runner;
  runner.addTest(suite);

  const bool sucessful = runner.run();
  return sucessful ? EXIT_SUCCESS : EXIT_FAILURE;
}
