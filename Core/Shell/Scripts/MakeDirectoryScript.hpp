/*
 * Core/Shell/Scripts/MakeDirectoryScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_MAKEDIRECTORYSCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_MAKEDIRECTORYSCRIPT_HPP_

#include "Shell/ArgParser.hpp"
#include "Shell/Settings.hpp"
#include "Shell/ShellScript.hpp"

class MakeDirectoryScript: public ShellScript
{
public:
  MakeDirectoryScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    ShellScript{parent, firstArgument, lastArgument}
  {
  }

  virtual Result run() override
  {
    static const ArgParser::Descriptor descriptors[] = {
        {"--help", nullptr, "show this help message and exit", 0, Arguments::helpSetter},
        {nullptr, "ENTRY", "make directory named ENTRY", 1, Arguments::pathSetter}
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
    else
    {
      return makeDirectory(arguments.path);
    }
  }

  static const char *name()
  {
    return "mkdir";
  }

private:
  struct Arguments
  {
    Arguments() :
      path{nullptr},
      help{false}
    {
    }

    const char *path;
    bool help;

    static void helpSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->help = true;
    }

    static void pathSetter(void *object, const char *argument)
    {
      static_cast<Arguments *>(object)->path = argument;
    }
  };

  Result makeDirectory(const char *positionalArgument)
  {
    Result res = E_OK;

    char absolutePath[Settings::PWD_LENGTH];
    ShellHelpers::joinPaths(absolutePath, env()["PWD"], positionalArgument);

    FsNode * const root = ShellHelpers::openBaseNode(fs(), absolutePath);
    if (root != nullptr)
    {
      FsNode * const node = ShellHelpers::openNode(fs(), absolutePath);
      if (node == nullptr)
      {
        const char * const nodeName = ShellHelpers::extractName(positionalArgument);
        const auto nodeTime = time().get();
        const FsFieldDescriptor fields[] = {
            // Name descriptor
            {
                nodeName,
                strlen(nodeName) + 1,
                FS_NODE_NAME
            },
            // Access time descriptor
            {
                &nodeTime,
                sizeof(nodeTime),
                FS_NODE_TIME
            }
        };

        res = fsNodeCreate(root, fields, ARRAY_SIZE(fields));
        if (res != E_OK)
          tty() << name() << ": " << positionalArgument << ": directory creation failed" << Terminal::EOL;
      }
      else
      {
        tty() << name() << ": " << positionalArgument << ": node already exists" << Terminal::EOL;
        fsNodeFree(node);
        res = E_EXIST;
      }

      fsNodeFree(root);
    }
    else
    {
      tty() << name() << ": " << positionalArgument << ": root node not found" << Terminal::EOL;
      res = E_ENTRY;
    }

    return res;
  }
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_MAKEDIRECTORYSCRIPT_HPP_
