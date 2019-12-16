/*
 * Core/Shell/Scripts/DisplayTestScript.hpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_DISPLAYTESTSCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_DISPLAYTESTSCRIPT_HPP_

#include <algorithm>
#include <xcore/memory.h>
#include "Shell/ArgParser.hpp"
#include "Shell/Scripts/DataReader.hpp"

template<size_t BUFFER_SIZE>
class DisplayTestScript: public DataReader
{
public:
  DisplayTestScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    DataReader{parent, firstArgument, lastArgument}
  {
  }

  virtual Result run() override
  {
    static const ArgParser::Descriptor descriptors[] = {
        {"--help", nullptr, "show this help message and exit", 0, Arguments::helpSetter},
        {"-w", "PIXELS", "display width", 1, Arguments::widthSetter},
        {"-h", "PIXELS", "display height", 1, Arguments::heightSetter},
        {nullptr, "DST", "device file", 1, Arguments::dstSetter}
    };

    bool argumentsParsed;
    const Arguments arguments = ArgParser::parse<Arguments>(m_firstArgument, m_lastArgument,
        std::cbegin(descriptors), std::cend(descriptors), &argumentsParsed);

    if (arguments.help)
    {
      ArgParser::help(tty(), name(), std::cbegin(descriptors), std::cend(descriptors));
      return E_OK;
    }
    else if (argumentsParsed)
    {
      return fillDisplay(arguments.dst, arguments.width, arguments.height);
    }
    else
    {
      return E_VALUE;
    }
  }

  static const char *name()
  {
    return "distest";
  }

private:
  struct RGB
  {
    uint8_t r;
    uint8_t g;
    uint8_t b;
  };

  static uint16_t rgbTo565(RGB color)
  {
    const uint16_t value = ((color.r >> 3) << 11) | ((color.g >> 2) << 5) | (color.b >> 3);
    return toBigEndian16(value);
  }

  static RGB hsvToRGB(uint16_t hue, uint8_t saturation, uint8_t value)
  {
    static const uint8_t mapping[6][3] = {
        {0, 2, 1},
        {3, 0, 1},
        {1, 0, 2},
        {1, 3, 0},
        {2, 1, 0},
        {0, 1, 3}
    };

    const uint16_t intValue = value * 100;
    const uint16_t minValue = (100 - saturation) * value;
    const uint16_t delta = ((intValue - minValue) * (hue % 60)) / 60;

    const uint16_t fill[4] = {
        intValue,
        minValue,
        static_cast<uint16_t>(minValue + delta),
        static_cast<uint16_t>(intValue - delta)
    };

    const uint8_t hi = (hue / 60) % 6;
    const uint8_t values[3] = {
        static_cast<uint8_t>(fill[mapping[hi][0]] * 255 / 10000),
        static_cast<uint8_t>(fill[mapping[hi][1]] * 255 / 10000),
        static_cast<uint8_t>(fill[mapping[hi][2]] * 255 / 10000)
    };

    return RGB{values[0], values[1], values[2]};
  }

  struct Arguments
  {
    const char *dst{nullptr};
    uint16_t height{};
    uint16_t width{};
    bool help{false};

    static void dstSetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->dst = argument;
    }

    static void heightSetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->height = static_cast<uint16_t>(atol(argument));
    }

    static void widthSetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->width = static_cast<uint16_t>(atol(argument));
    }

    static void helpSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->help = true;
    }
  };

  Result fillDisplay(const char *dst, uint16_t width, uint16_t height)
  {
    const size_t bufferedRowCount = BUFFER_SIZE / width / 2;

    // Check input variables
    if (!bufferedRowCount)
      return E_VALUE;

    const size_t chunkSize = bufferedRowCount * width * 2;
    Result res;

    // Open the destination node
    FsNode * const dstNode = ShellHelpers::openSink(fs(), env(), time(), dst, true, &res);
    if (dstNode == nullptr)
    {
      tty() << name() << ": " << dst << ": open failed" << Terminal::EOL;
      return res;
    }

    // Copy data
    FsLength position = 0;
    uint16_t buffer[BUFFER_SIZE / 2];

    for (size_t row = 0; row < height;)
    {
      for (size_t i = 0; i < std::min(bufferedRowCount, height - row); ++i)
      {
        const uint16_t hue = (row + i) * 360 / height;

        for (size_t j = 0; j < width; ++j)
        {
          const uint8_t value = 100 - j * 100 / width;
          buffer[i * width + j] = rgbTo565(hsvToRGB(hue, 100, value));
        }
      }

      size_t bytesWritten;

      res = fsNodeWrite(dstNode, FS_NODE_DATA, position, buffer, chunkSize, &bytesWritten);
      if (res != E_OK || bytesWritten != chunkSize)
      {
        tty() << name() << ": write error at " << position << Terminal::EOL;

        if (res == E_OK)
          res = E_FULL;
        break;
      }

      position += chunkSize;
      row += bufferedRowCount;
    }

    fsNodeFree(dstNode);
    return res;
  }
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_DISPLAYTESTSCRIPT_HPP_
