/*
 * Core/Shell/Scripts/DirectDataScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_DIRECTDATASCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_DIRECTDATASCRIPT_HPP_

#include "Shell/ArgParser.hpp"
#include "Shell/Scripts/DataReader.hpp"

template<size_t BUFFER_SIZE>
class DirectDataScript: public DataReader
{
public:
  DirectDataScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    DataReader{parent, firstArgument, lastArgument}
  {
  }

  virtual Result run() override
  {
    // Parse and verify arguments
    static const ArgParser::Descriptor descriptors[] = {
        {"--help", nullptr, "show this help message and exit", 0, Arguments::helpSetter},
        {"--if", "FILE", "read from FILE", 1, Arguments::srcSetter},
        {"--of", "FILE", "write to FILE", 1, Arguments::dstSetter},
        {"--bs", "BYTES", "read and write up to BYTES at a time", 1, Arguments::bsSetter},
        {"--count", "N", "copy only N input blocks", 1, Arguments::countSetter},
        {"--seek", "N", "skip N blocks at start of output", 1, Arguments::seekSetter},
        {"--skip", "N", "skip N blocks at start of input", 1, Arguments::skipSetter}
    };

    bool argumentsParsed;
    const Arguments arguments = ArgParser::parse<Arguments>(m_firstArgument, m_lastArgument,
        std::cbegin(descriptors), std::cend(descriptors), &argumentsParsed);

    if (arguments.help)
    {
      ArgParser::help(tty(), name(), std::cbegin(descriptors), std::cend(descriptors));
      return E_OK;
    }
    else if (!argumentsParsed)
    {
      tty() << name() << ": incorrect arguments" << Terminal::EOL;
      return E_VALUE;
    }
    else if (arguments.src == nullptr || arguments.dst == nullptr)
    {
      tty() << name() << ": source and destination files should be provided" << Terminal::EOL;
      return E_VALUE;
    }
    else if (arguments.bs > BUFFER_SIZE)
    {
      tty() << name() << ": block size is too big: " << arguments.bs << ", available " << BUFFER_SIZE << Terminal::EOL;
      return E_VALUE;
    }

    // Open the source node
    FsNode * const src = ShellHelpers::openSource(fs(), env(), arguments.src);
    if (src == nullptr)
    {
      tty() << name() << ": " << arguments.src << ": open failed" << Terminal::EOL;
      return E_ENTRY;
    }

    // Open the destination node
    Result res;
    FsNode * const dst = ShellHelpers::openSink(fs(), env(), time(), arguments.dst, false, &res);
    if (dst == nullptr)
    {
      tty() << name() << ": " << arguments.dst << ": open failed" << Terminal::EOL;
      fsNodeFree(src);
      return res;
    }

    // Copy data
    uint8_t buffer[BUFFER_SIZE];
    FsLength pos = static_cast<FsLength>(arguments.seek) * arguments.bs;

    res = read(buffer, src, arguments.bs, 0, 0, std::bind(&DirectDataScript::onDataRead, this, dst, &pos,
        std::placeholders::_1, std::placeholders::_2));

    fsNodeFree(dst);
    fsNodeFree(src);

    return res;
  }

  static const char *name()
  {
    return "dd";
  }

private:
  struct Arguments
  {
    Arguments() :
      src{nullptr},
      dst{nullptr},
      bs{BUFFER_SIZE},
      count{0},
      seek{0},
      skip{0},
      help{false}
    {
    }

    const char *src;
    const char *dst;
    size_t bs;
    size_t count;
    size_t seek;
    size_t skip;
    bool help;

    static void srcSetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->src = argument;
    }

    static void dstSetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->dst = argument;
    }

    static void bsSetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->bs = static_cast<size_t>(atol(argument));
    }

    static void countSetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->count = static_cast<size_t>(atol(argument));
    }

    static void seekSetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->seek = static_cast<size_t>(atol(argument));
    }

    static void skipSetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->skip = static_cast<size_t>(atol(argument));
    }

    static void helpSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->help = true;
    }
  };

  Result onDataRead(FsNode *dst, FsLength *position, const void *buffer, size_t bytesRead)
  {
    size_t bytesWritten;
    Result res = fsNodeWrite(dst, FS_NODE_DATA, *position, buffer, bytesRead, &bytesWritten);

    if (res != E_OK || bytesRead != bytesWritten)
    {
      tty() << "write error at " << *position << Terminal::EOL;

      if (res == E_OK)
        res = E_ERROR;
    }
    else
      *position += bytesWritten;

    return res;
  }
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_DIRECTDATASCRIPT_HPP_
