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
    Arguments arguments;
    const std::array<ArgParser::Descriptor, 2> descriptors = {
        {
            {"--help", "print help message", 0, boolArgumentSetter, &arguments.showHelpMessage},
            {nullptr, nullptr, 2, positionalArgumentParser, &arguments}
        }
    };

    ArgParser::parse(m_firstArgument, m_lastArgument, descriptors.begin(), descriptors.end());

    if (arguments.src == nullptr || arguments.dst == nullptr || arguments.showHelpMessage)
    {
      ArgParser::help(tty(), descriptors.begin(), descriptors.end());
      return E_OK;
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
    FsLength pos = 0;

    using namespace std::placeholders;
    res = read(buffer, src, BUFFER_SIZE, 0, 0, std::bind(&CopyNodeScript::onDataRead, this, dst, &pos, _1, _2));

    fsNodeFree(dst);
    fsNodeFree(src);

    return res;
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
      showHelpMessage{false}
    {
    }

    const char *src;
    const char *dst;
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

  static void positionalArgumentParser(void *object, const char *argument)
  {
    auto * const args = static_cast<Arguments *>(object);

    if (args->src == nullptr)
      args->src = argument;
    else if (args->dst == nullptr)
      args->dst = argument;
    else
      args->showHelpMessage = true; // Incorrect argument count
  }
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_COPYNODESCRIPT_HPP_
