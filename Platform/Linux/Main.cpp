/*
 * Main.cpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "MmfBuilder.hpp"
#include "UnixTimeProvider.hpp"

#include "Shell/Initializer.hpp"
#include "Shell/Interfaces/InterfaceNode.hpp"
#include "Shell/Scripts/ChangeDirectoryScript.hpp"
#include "Shell/Scripts/ChangeModeScript.hpp"
#include "Shell/Scripts/ChecksumCrc32Script.hpp"
#include "Shell/Scripts/CopyNodeScript.hpp"
#include "Shell/Scripts/DateScript.hpp"
#include "Shell/Scripts/DirectDataScript.hpp"
#include "Shell/Scripts/EchoScript.hpp"
#include "Shell/Scripts/ExitScript.hpp"
#include "Shell/Scripts/GetEnvScript.hpp"
#include "Shell/Scripts/HelpScript.hpp"
#include "Shell/Scripts/ListEnvScript.hpp"
#include "Shell/Scripts/ListNodesScript.hpp"
#include "Shell/Scripts/MakeDirectoryScript.hpp"
#include "Shell/Scripts/MountScript.hpp"
#include "Shell/Scripts/PrintHexDataScript.hpp"
#include "Shell/Scripts/PrintRawDataScript.hpp"
#include "Shell/Scripts/RemoveNodesScript.hpp"
#include "Shell/Scripts/SetEnvScript.hpp"
#include "Shell/Scripts/Shell.hpp"
#include "Shell/Scripts/TimeScript.hpp"
#include "Shell/SerialTerminal.hpp"
#include "Vfs/VfsHandle.hpp"

#include <halm/platform/generic/console.h>
#include <xcore/os/thread.h>

#include <iostream>
#include <uv.h>

class Application
{
public:
  Application(Interface *serial, bool echoing = true, char **partitions = nullptr, size_t count = 0) :
    m_serial{serial, [](Interface *pointer){ deinit(pointer); raise(SIGUSR1); }},
    m_filesystem{static_cast<FsHandle *>(init(VfsHandleClass, nullptr)), [](FsHandle *pointer){ deinit(pointer); }},
    m_terminal{m_serial.get()},
    m_initializer{m_filesystem.get(), m_terminal, UnixTimeProvider::instance(), echoing},
    m_count{count},
    m_partitions{partitions}
  {
    if (m_serial == nullptr || m_filesystem == nullptr)
      abort(); // TODO Rewrite
  }

  Application(char **partitions = nullptr, size_t count = 0) :
    Application{static_cast<Interface *>(init(Console, nullptr)), true, partitions, count}
  {
  }

  int run()
  {
    bootstrap(m_partitions, m_count);

    m_initializer.attach<ChangeDirectoryScript>();
    m_initializer.attach<ChangeModeScript>();
    m_initializer.attach<ChecksumCrc32Script<BUFFER_SIZE>>();
    m_initializer.attach<CopyNodeScript<BUFFER_SIZE>>();
    m_initializer.attach<DateScript>();
    m_initializer.attach<DirectDataScript<BUFFER_SIZE>>();
    m_initializer.attach<EchoScript>();
    m_initializer.attach<ExitScript>();
    m_initializer.attach<GetEnvScript>();
    m_initializer.attach<HelpScript>();
    m_initializer.attach<ListEnvScript>();
    m_initializer.attach<ListNodesScript>();
    m_initializer.attach<MakeDirectoryScript>();
    m_initializer.attach<MountScript<>>();
    m_initializer.attach<PrintHexDataScript<BUFFER_SIZE>>();
    m_initializer.attach<PrintRawDataScript<BUFFER_SIZE>>();
    m_initializer.attach<RemoveNodesScript>();
    m_initializer.attach<SetEnvScript>();
    m_initializer.attach<Shell>();
    m_initializer.attach<TimeScript>();

    return m_initializer.run() == E_OK ? EXIT_SUCCESS : EXIT_FAILURE;
  }

private:
  static constexpr size_t BUFFER_SIZE{4096};

  std::unique_ptr<Interface, std::function<void (Interface *)>> m_serial;
  std::unique_ptr<FsHandle, std::function<void (FsHandle *)>> m_filesystem;
  SerialTerminal m_terminal;
  Initializer m_initializer;

  size_t m_count;
  char **m_partitions;

  void bootstrap(char **partitions = nullptr, size_t count = 0)
  {
    VfsNode *node;

    // Root nodes
    node = new VfsDirectory{UnixTimeProvider::instance().getTime()};
    ShellHelpers::injectNode(m_filesystem.get(), node, "/bin");

    node = new VfsDirectory{UnixTimeProvider::instance().getTime()};
    ShellHelpers::injectNode(m_filesystem.get(), node, "/dev");

    for (size_t i = 0; i < std::max(count, static_cast<size_t>('z' - 'a')); ++i)
    {
      const std::string path = std::string{"/dev/sd"} + static_cast<char>('a' + i);
      Interface * const mmf = MmfBuilder::build(partitions[i]);

      if (mmf != nullptr)
      {
        node = new InterfaceNode<>{mmf};
        ShellHelpers::injectNode(m_filesystem.get(), node, path.data());
      }
    }
  }
};

void applicationWrapper(void *argument)
{
  Application * const application = static_cast<Application *>(argument);
  application->run();
  delete application;
}

static void userSignalCallback(uv_signal_t *, int)
{
  uv_stop(uv_default_loop());
}

int main(int argc, char *argv[])
{
  bool help = false;

  for (int i = 1; i < argc; ++i)
  {
    if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h"))
    {
      help = true;
      continue;
    }
  }

  if (help)
  {
    std::cout << "Usage: shell [OPTION]... FILE" << std::endl;
    std::cout << "  -h, --help  print help message" << std::endl;
    exit(EXIT_SUCCESS);
  }
  else
  {
    const auto loop = uv_default_loop();
    uv_signal_t listener;

    uv_signal_init(loop, &listener);
    uv_signal_start(&listener, userSignalCallback, SIGUSR1);

    Application * const application = new Application(argv + 1, static_cast<size_t>(argc - 1));
    Thread appThread;
    threadInit(&appThread, 4096, 0, applicationWrapper, application);
    threadStart(&appThread);

    uv_run(loop, UV_RUN_DEFAULT);
    return EXIT_SUCCESS;
  }
}
