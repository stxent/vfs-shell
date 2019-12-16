/*
 * Platform/LPC17xx_DevKit/Scripts/ShutdownScript.hpp
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_SCRIPTS_SHUTDOWNSCRIPT_HPP_
#define VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_SCRIPTS_SHUTDOWNSCRIPT_HPP_

#include <halm/core/cortex/nvic.h>
#include <halm/platform/nxp/backup_domain.h>
#include <halm/pm.h>
#include "Shell/ShellScript.hpp"

class ShutdownScript: public ShellScript
{
public:
  ShutdownScript(Script *parent, ArgumentIterator firstArgument, ArgumentIterator lastArgument) :
    ShellScript{parent, firstArgument, lastArgument}
  {
  }

  virtual Result run() override
  {
    static const ArgParser::Descriptor descriptors[] = {
        {"--help", nullptr, "show this help message and exit", 0, Arguments::helpSetter},
        {"-h", nullptr, "halt system", 0, Arguments::haltSetter},
        {"-r", nullptr, "restart system", 0, Arguments::restartSetter}
    };

    const Arguments arguments = ArgParser::parse<Arguments>(m_firstArgument, m_lastArgument,
        std::cbegin(descriptors), std::cend(descriptors));

    if (arguments.help || (!arguments.halt && !arguments.restart))
    {
      ArgParser::help(tty(), name(), std::cbegin(descriptors), std::cend(descriptors));
      return E_OK;
    }
    else if (arguments.halt || arguments.restart)
    {
      static_cast<uint32_t *>(backupDomainAddress())[0] = MAGIC_WORD;

      if (arguments.restart)
        nvicResetCore();
      else
        pmChangeState(PM_SHUTDOWN);

      return E_OK; // Unreachable code
    }
    else
    {
      return E_VALUE;
    }
  }

  static bool isRestarted()
  {
    return static_cast<const uint32_t *>(backupDomainAddress())[0] == MAGIC_WORD;
  }

  static const char *name()
  {
    return "shutdown";
  }

private:
  static constexpr uint32_t MAGIC_WORD{0x3A84508FUL};

  struct Arguments
  {
    bool halt{false};
    bool help{false};
    bool restart{false};

    static void haltSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->halt = true;
    }

    static void helpSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->help = true;
    }

    static void restartSetter(void *object, const char *)
    {
      static_cast<Arguments *>(object)->restart = true;
    }
  };
};

#endif // VFS_SHELL_PLATFORM_LPC17XX_DEVKIT_SCRIPTS_SHUTDOWNSCRIPT_HPP_
