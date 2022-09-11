/*
 * Tests/InterfaceNode/MockDisplay.hpp
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_TESTS_INTERFACENODE_MOCKDISPLAY_HPP_
#define VFS_SHELL_TESTS_INTERFACENODE_MOCKDISPLAY_HPP_

#include <dpm/displays/display.h>
#include <new>

extern const InterfaceClass * const MockDisplay;

class MockDisplay
{
public:
  MockDisplay(const MockDisplay &) = delete;
  MockDisplay &operator=(const MockDisplay &) = delete;

  static Result init(void *object, const void *)
  {
    new (object) MockDisplay{};
    return E_OK;
  }

  static void deinit(void *object)
  {
    static_cast<MockDisplay *>(object)->~MockDisplay();
  }

  static Result getParam(void *object, int parameter, void *data)
  {
    return static_cast<MockDisplay *>(object)->getParamImpl(parameter, data);
  }

  static Result setParam(void *object, int parameter, const void *data)
  {
    return static_cast<MockDisplay *>(object)->setParamImpl(parameter, data);
  }

  static size_t read(void *object, void *buffer, size_t length)
  {
    return static_cast<MockDisplay *>(object)->readImpl(buffer, length);
  }

  static size_t write(void *object, const void *buffer, size_t length)
  {
    return static_cast<MockDisplay *>(object)->writeImpl(buffer, length);
  }

private:
  static constexpr size_t DISPLAY_HEIGHT{24};
  static constexpr size_t DISPLAY_WIDTH{32};

  Interface m_base;

  DisplayResolution m_resolution{};
  DisplayWindow m_window{};
  DisplayOrientation m_orientation{};
  size_t m_position{0};
  uint8_t m_data[DISPLAY_HEIGHT * DISPLAY_WIDTH];

  MockDisplay();
  ~MockDisplay();

  Result getParamImpl(int, void *);
  Result setParamImpl(int, const void *);
  size_t readImpl(void *, size_t);
  size_t writeImpl(const void *, size_t);
};

#endif // VFS_SHELL_TESTS_INTERFACENODE_MOCKDISPLAY_HPP_
