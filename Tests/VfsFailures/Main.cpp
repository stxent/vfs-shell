/*
 * Main.cpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "Shell/ShellHelpers.hpp"
#include "Vfs/VfsDataNode.hpp"
#include "Vfs/VfsDirectory.hpp"
#include "Vfs/VfsHandle.hpp"
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

extern "C" void *__libc_malloc(size_t);

static unsigned int mallocHookFails = 0;

extern "C" void *malloc(size_t size)
{
  bool allocate = true;

  if (mallocHookFails && !--mallocHookFails)
    allocate = false;

  return allocate ? __libc_malloc(size) : nullptr;
}

class VfsTest: public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(VfsTest);
  CPPUNIT_TEST(testDataNodeFailures);
  CPPUNIT_TEST(testNodeCreationFailures);
  CPPUNIT_TEST(testNodeInjection);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testDataNodeFailures();
  void testNodeCreationFailures();
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

void VfsTest::testDataNodeFailures()
{
  VfsDataNode * const node = new VfsDataNode{};
  CPPUNIT_ASSERT(node != nullptr);

  Result res;
  bool ok;

  mallocHookFails = 1;
  ok = node->rename("test");
  CPPUNIT_ASSERT(ok == false);

  mallocHookFails = 1;
  ok = node->reserve(1000, '\0');
  CPPUNIT_ASSERT(ok == false);

  mallocHookFails = 1;
  ok = node->reserve("test pattern", 12);
  CPPUNIT_ASSERT(ok == false);

  const uint8_t bufferData[BUFFER_SIZE]{};

  mallocHookFails = 1;
  res = node->write(FS_NODE_DATA, 0, &bufferData, sizeof(bufferData), nullptr);
  CPPUNIT_ASSERT(res == E_MEMORY);

  delete node;
}

void VfsTest::testNodeCreationFailures()
{
  auto dir = new VfsDirectory{};
  CPPUNIT_ASSERT(dir != nullptr);
  auto node = new VfsDataNode{};
  CPPUNIT_ASSERT(node != nullptr);

  // Try to create node, simulate renaming failure

  const std::array<FsFieldDescriptor, 2> memoryFailureTest = {{
      {
          "test",
          strlen("test") + 1,
          FS_NODE_NAME
      }, {
          nullptr,
          0,
          FS_NODE_DATA
      }
  }};

  mallocHookFails = 2;
  const auto res = dir->create(memoryFailureTest.data(), memoryFailureTest.size());
  CPPUNIT_ASSERT(res == E_MEMORY);

  delete node;
  delete dir;
}

void VfsTest::testNodeInjection()
{
  auto node = new VfsDataNode{};
  CPPUNIT_ASSERT(node != nullptr);

  // Test rename failure
  mallocHookFails = 1;
  const auto res = ShellHelpers::injectNode(handle, node, "/test");
  CPPUNIT_ASSERT(res == E_MEMORY);

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
