/*
 * Core/Shell/Scripts/CopyNodeScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_COPYNODESCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_COPYNODESCRIPT_HPP_

#include "Shell/ArgParser.hpp"
#include "Shell/Scripts/DataReader.hpp"

template<size_t BUFFER_SIZE>
class CopyNodeScript: public DataReader
{
public:
  CopyNodeScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    DataReader{parent, firstArgument, lastArgument}
  {
  }

  virtual Result run() override
  {
    static const ArgParser::Descriptor descriptors[] = {
        {"--help", nullptr, "show this help message and exit", 0, Arguments::helpSetter},
        {nullptr, "SRC", "source file", 1, Arguments::srcSetter},
        {nullptr, "DST", "destination file or directory", 1, Arguments::dstSetter}
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
      return copyNodes(arguments.src, arguments.dst);
    }
    else
    {
      return E_VALUE;
    }
  }

  static const char *name()
  {
    return "cp";
  }

private:
  struct Arguments
  {
    Arguments() :
      src{nullptr},
      dst{nullptr},
      help{false}
    {
    }

    const char *src;
    const char *dst;
    bool help;

    static void srcSetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->src = argument;
    }

    static void dstSetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->dst = argument;
    }

    static void helpSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->help = true;
    }
  };

  Result copyNodes(const char *src, const char *dst)
  {
    // Open the source node
    FsNode * const srcNode = ShellHelpers::openSource(fs(), env(), src);
    if (srcNode == nullptr)
    {
      tty() << name() << ": " << src << ": open failed" << Terminal::EOL;
      return E_ENTRY;
    }

    // TODO Check whether the destination is an existing directory

    // Open the destination node
    Result res;
    FsNode * const dstNode = ShellHelpers::openSink(fs(), env(), time(), dst, false, &res);
    if (dstNode == nullptr)
    {
      tty() << name() << ": " << dst << ": open failed" << Terminal::EOL;
      fsNodeFree(srcNode);
      return res;
    }

    // Copy data
    uint8_t buffer[BUFFER_SIZE];
    FsLength pos = 0;

    res = read(buffer, srcNode, BUFFER_SIZE, 0, 0,
        [this, dstNode, &pos](const void *buf, size_t len){ return onDataRead(dstNode, &pos, buf, len); });

    fsNodeFree(dstNode);
    fsNodeFree(srcNode);

    return res;
  }

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

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_COPYNODESCRIPT_HPP_
