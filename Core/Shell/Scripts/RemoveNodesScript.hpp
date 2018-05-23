/*
 * Core/Shell/Scripts/RemoveNodesScript.hpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_CORE_SHELL_SCRIPTS_REMOVENODESSCRIPT_HPP_
#define VFS_SHELL_CORE_SHELL_SCRIPTS_REMOVENODESSCRIPT_HPP_

#include "Shell/ArgParser.hpp"
#include "Shell/Settings.hpp"
#include "Shell/ShellScript.hpp"

class RemoveNodesScript: public ShellScript
{
public:
  RemoveNodesScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    ShellScript{parent, firstArgument, lastArgument},
    m_result{E_OK}
  {
  }

  virtual Result run() override
  {
    static const ArgParser::Descriptor descriptors[] = {
        {"--help", nullptr, "show this help message and exit", 0, Arguments::helpSetter},
        {"-r", nullptr, "remove recursively", 0, Arguments::recursiveSetter},
        {nullptr, "ENTRY", "remove ENTRY", 0, nullptr}
    };

    const Arguments arguments = ArgParser::parse<Arguments>(m_firstArgument, m_lastArgument,
        std::cbegin(descriptors), std::cend(descriptors));

    if (arguments.help)
    {
      ArgParser::help(tty(), name(), std::cbegin(descriptors), std::cend(descriptors));
      return E_OK;
    }
    else
    {
      ArgParser::invoke(m_firstArgument, m_lastArgument, std::cbegin(descriptors), std::cend(descriptors),
          [this, &arguments](const char *key){ removeNode(key, arguments.recursive); });
      return m_result;
    }
  }

  static const char *name()
  {
    return "rm";
  }

private:
  struct Arguments
  {
    Arguments() :
      help{false},
      recursive{false}
    {
    }

    bool help;
    bool recursive;

    static void helpSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->help = true;
    }

    static void recursiveSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->recursive = true;
    }
  };

  Result m_result;

  void removeNode(const char *positionalArgument, bool recursive)
  {
    if (m_result != E_OK)
      return;

    char absolutePath[Settings::PWD_LENGTH];
    ShellHelpers::joinPaths(absolutePath, env()["PWD"], positionalArgument);

    FsNode * const node = ShellHelpers::openNode(fs(), absolutePath);
    if (node != nullptr)
    {
      // TODO Remove directories recursively
      if (recursive || fsNodeLength(node, FS_NODE_DATA, nullptr) == E_OK)
      {
        FsNode * const root = ShellHelpers::openBaseNode(fs(), absolutePath);
        if (root != nullptr)
        {
          m_result = fsNodeRemove(root, node);

          if (m_result != E_OK)
            tty() << name() << ": " << positionalArgument << ": deletion failed" << Terminal::EOL;

          fsNodeFree(root);
        }
        else
        {
          tty() << name() << ": " << positionalArgument << ": root node not found" << Terminal::EOL;
          m_result = E_ENTRY;
        }
      }
      else
      {
        // Directory node is ignored
        tty() << name() << ": " << positionalArgument << ": hollow node ignored" << Terminal::EOL;
        m_result = E_ENTRY;
      }

      fsNodeFree(node);
    }
    else
    {
      tty() << name() << ": " << positionalArgument << ": node not found" << Terminal::EOL;
      m_result = E_ENTRY;
    }
  }
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_REMOVENODESSCRIPT_HPP_
