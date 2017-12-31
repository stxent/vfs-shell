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
    Arguments arguments;
    const std::array<ArgParser::Descriptor, 7> descriptors = {
        {
            {"--help", "print help message", 0, boolArgumentSetter, &arguments.showHelpMessage},
            {"--if", "read from FILE", 1, stringArgumentSetter, &arguments.src},
            {"--of", "write to FILE", 1, stringArgumentSetter, &arguments.dst},
            {"--bs", "read and write up to BYTES at a time", 1, sizeArgumentSetter, &arguments.bs},
            {"--count", "copy only COUNT input blocks", 1, sizeArgumentSetter, &arguments.count},
            {"--seek", "skip COUNT blocks at start of output", 1, sizeArgumentSetter, &arguments.seek},
            {"--skip", "skip COUNT blocks at start of input", 1, sizeArgumentSetter, &arguments.skip}
        }
    };

    ArgParser::parse(m_firstArgument, m_lastArgument, descriptors.begin(), descriptors.end());

    if (arguments.src == nullptr || arguments.dst == nullptr || arguments.showHelpMessage)
    {
      ArgParser::help(tty(), descriptors.begin(), descriptors.end());
      return E_OK;
    }
    if (arguments.bs > BUFFER_SIZE)
    {
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

    using namespace std::placeholders;
    res = read(buffer, src, arguments.bs, 0, 0, std::bind(&DirectDataScript::onDataRead, this, dst, &pos, _1, _2));

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
      showHelpMessage{false}
    {
    }

    const char *src;
    const char *dst;
    size_t bs;
    size_t count;
    size_t seek;
    size_t skip;
    bool showHelpMessage;
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

  static void boolArgumentSetter(void *object, const char *)
  {
    *static_cast<bool *>(object) = true;
  }

  static void sizeArgumentSetter(void *object, const char *argument)
  {
    *static_cast<size_t *>(object) = static_cast<size_t>(atol(argument));
  }

  static void stringArgumentSetter(void *object, const char *argument)
  {
    *static_cast<const char **>(object) = argument;
  }
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_DIRECTDATASCRIPT_HPP_
