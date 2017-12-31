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
    Arguments arguments;
    const std::array<ArgParser::Descriptor, 2> descriptors{
        {
            {"-r", "remove recursively", 0, boolArgumentSetter, &arguments.removeRecursively},
            {"--help", "print help message", 0, boolArgumentSetter, &arguments.showHelpMessage}
        }
    };

    ArgParser::parse(m_firstArgument, m_lastArgument, descriptors.begin(), descriptors.end());

    if (arguments.showHelpMessage)
    {
      ArgParser::help(tty(), descriptors.begin(), descriptors.end());
      return E_OK;
    }
    else
    {
      ArgParser::invoke(m_firstArgument, m_lastArgument, descriptors.begin(), descriptors.end(),
          std::bind(&RemoveNodesScript::removeNode, this, std::placeholders::_1, arguments.removeRecursively));
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
      removeRecursively{false},
      showHelpMessage{false}
    {
    }

    bool removeRecursively;
    bool showHelpMessage;
  };

  Result m_result;

  void removeNode(const char *positionalArgument, bool /*recursive*/)
  {
    if (m_result != E_OK)
      return;

    char absolutePath[Settings::PWD_LENGTH];
    ShellHelpers::joinPaths(absolutePath, env()["PWD"], positionalArgument);

    FsNode * const node = ShellHelpers::openNode(fs(), absolutePath);
    if (node != nullptr)
    {
      if (fsNodeLength(node, FS_NODE_DATA, nullptr) == E_OK)
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

  static void boolArgumentSetter(void *object, const char *)
  {
    *static_cast<bool *>(object) = true;
  }
};

#endif // VFS_SHELL_CORE_SHELL_SCRIPTS_REMOVENODESSCRIPT_HPP_
