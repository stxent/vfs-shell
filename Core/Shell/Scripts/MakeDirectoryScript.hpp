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
    ShellScript{parent, firstArgument, lastArgument},
    m_result{E_OK}
  {
  }

  virtual Result run() override
  {
    Arguments arguments;
    const std::array<ArgParser::Descriptor, 2> descriptors{
        {
            {"--help", "print help message", 0, boolArgumentSetter, &arguments.showHelpMessage},
            {nullptr, nullptr, 1, positionalArgumentParser, &arguments}
        }
    };

    ArgParser::parse(m_firstArgument, m_lastArgument, descriptors.begin(), descriptors.end());

    if (arguments.showHelpMessage || arguments.target == nullptr)
    {
      ArgParser::help(tty(), descriptors.begin(), descriptors.end());
      return E_OK;
    }
    else
      return makeDirectory(arguments.target);
  }

  static const char *name()
  {
    return "mkdir";
  }

private:
  struct Arguments
  {
    Arguments() :
      target{nullptr},
      showHelpMessage{false}
    {
    }

    const char *target;
    bool showHelpMessage;
  };

  Result m_result;

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
        const auto nodeTime = time().microtime();
        const FsFieldDescriptor descriptors[] = {
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

        res = fsNodeCreate(root, descriptors, ARRAY_SIZE(descriptors));
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

  static void boolArgumentSetter(void *object, const char *)
  {
    *static_cast<bool *>(object) = true;
  }

  static void positionalArgumentParser(void *object, const char *argument)
  {
    auto * const args = static_cast<Arguments *>(object);

    if (args->target == nullptr)
      args->target = argument;
    else
      args->showHelpMessage = true; // Incorrect argument count
  }
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_MAKEDIRECTORYSCRIPT_HPP_
