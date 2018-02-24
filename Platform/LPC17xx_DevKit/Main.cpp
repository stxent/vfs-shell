/*
 * Main.cpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <cassert>

#include <halm/core/cortex/nvic.h>
#include <halm/generic/sdio_spi.h>
#include <halm/pin.h>
#include <halm/platform/nxp/gptimer.h>
#include <halm/platform/nxp/lpc17xx/clocking.h>
#include <halm/platform/nxp/serial.h>
#include <halm/platform/nxp/spi_dma.h>
#include <halm/pm.h>
#include <dpm/drivers/displays/display.h>
#include <dpm/drivers/displays/s6d1121.h>
#include <dpm/drivers/platform/nxp/memory_bus_dma.h>

#include "Shell/Initializer.hpp"
#include "Shell/Interfaces/InterfaceNode.hpp"
#include "Shell/MockTimeProvider.hpp"
#include "Shell/Scripts/ChangeDirectoryScript.hpp"
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
#include "Shell/Scripts/PrintHexDataScript.hpp"
#include "Shell/Scripts/PrintRawDataScript.hpp"
#include "Shell/Scripts/RemoveNodesScript.hpp"
#include "Shell/Scripts/SetEnvScript.hpp"
#include "Shell/Scripts/Shell.hpp"
#include "Shell/Scripts/TimeScript.hpp"
#include "Shell/SerialTerminal.hpp"
#include "Vfs/VfsHandle.hpp"

#include "CardBuilder.hpp"
#include "InterfaceWrapper.hpp"
#include "Scripts/CpuFrequencyScript.hpp"
#include "Scripts/MakeDacScript.hpp"
#include "Scripts/MakePinScript.hpp"
#include "Scripts/MountScript.hpp"
#include "Scripts/ShutdownScript.hpp"
#include "RealTimeClock.hpp"

static void enableClock()
{
  static const ExternalOscConfig extOscConfig = {12000000, false};
  static const PllConfig sysPllConfig = {CLOCK_EXTERNAL, 3, 25};
  static const GenericClockConfig mainClockConfig = {CLOCK_PLL};

  clockEnable(ExternalOsc, &extOscConfig);
  while (!clockReady(ExternalOsc));

  clockEnable(SystemPll, &sysPllConfig);
  while (!clockReady(SystemPll));

  clockEnable(MainClock, &mainClockConfig);
  while (!clockReady(MainClock));
}

class Application
{
private:
  static constexpr PinNumber DISPLAY_CS     = PIN(2, 8);
  static constexpr PinNumber DISPLAY_RESET  = PIN(1, 4);
  static constexpr PinNumber DISPLAY_RS     = PIN(1, 14);

  static constexpr PinNumber LED_B          = PIN(1, 8);
  static constexpr PinNumber LED_G          = PIN(1, 9);
  static constexpr PinNumber LED_R          = PIN(1, 10);
  static constexpr PinNumber SDIO_CS_PIN    = PIN(0, 22);

  static const MemoryBusDmaConfig s_displayBusConfig;
  static const PinNumber s_displayBusPins[];
  static const GpTimerConfig s_sdioTimerConfig;
  static const SerialConfig s_serialConfig;
  static const SpiDmaConfig s_spiConfig;

  static void interfaceDeleter(Interface *pointer)
  {
    deinit(pointer);
  }

public:
  Application() :
    m_displayBus{static_cast<Interface *>(init(MemoryBusDma, &s_displayBusConfig)), interfaceDeleter},
    m_display{makeDisplayInterface(m_displayBus.get()), interfaceDeleter},
    m_serial{static_cast<Interface *>(init(Serial, &s_serialConfig)), interfaceDeleter},
    m_spi{static_cast<Interface *>(init(SpiDma, &s_spiConfig)), interfaceDeleter},
    m_sdioTimer{static_cast<Timer *>(init(GpTimer, &s_sdioTimerConfig)), [](Timer *pointer){ deinit(pointer); }},
    m_sdio{makeSdioInterface(m_spi.get(), m_sdioTimer.get(), SDIO_CS_PIN), interfaceDeleter},
    m_sdioWrapper{makeSdioWrapper(m_sdio.get(), LED_G, LED_B), interfaceDeleter},
    m_filesystem{static_cast<FsHandle *>(init(VfsHandleClass, nullptr)), [](FsHandle *pointer){ deinit(pointer); }},
    m_terminal{m_serial.get()},
    m_initializer{m_filesystem.get(), m_terminal, RealTimeClock::instance(), true}
  {
    timerSetOverflow(m_sdioTimer.get(), 20); // 5 kHz SDIO update rate
  }

  int run()
  {
    bootstrap();

    if (ShutdownScript::isRestarted())
    {
      m_terminal << "System restarted" << Terminal::EOL;
    }

    m_initializer.attach<ChangeDirectoryScript>();
    m_initializer.attach<ChecksumCrc32Script<BUFFER_SIZE>>();
    m_initializer.attach<CopyNodeScript<BUFFER_SIZE>>();
    m_initializer.attach<CpuFrequencyScript<12000000>>();
    m_initializer.attach<DateScript>();
    m_initializer.attach<DirectDataScript<BUFFER_SIZE>>();
    m_initializer.attach<EchoScript>();
    m_initializer.attach<ExitScript>();
    m_initializer.attach<GetEnvScript>();
    m_initializer.attach<HelpScript>();
    m_initializer.attach<ListEnvScript>();
    m_initializer.attach<ListNodesScript>();
    m_initializer.attach<MakeDacScript>();
    m_initializer.attach<MakeDirectoryScript>();
    m_initializer.attach<MakePinScript>();
    m_initializer.attach<MountScript<CardBuilder>>(m_sdioWrapper.get());
    m_initializer.attach<PrintHexDataScript<BUFFER_SIZE>>();
    m_initializer.attach<PrintRawDataScript<BUFFER_SIZE>>();
    m_initializer.attach<RemoveNodesScript>();
    m_initializer.attach<SetEnvScript>();
    m_initializer.attach<Shell>();
    m_initializer.attach<ShutdownScript>();
    m_initializer.attach<TimeScript>();

    return m_initializer.run() == E_OK ? EXIT_SUCCESS : EXIT_FAILURE;
  }

private:
  static constexpr size_t BUFFER_SIZE = 2048;

  std::unique_ptr<Interface, std::function<void (Interface *)>> m_displayBus;
  std::unique_ptr<Interface, std::function<void (Interface *)>> m_display;
  std::unique_ptr<Interface, std::function<void (Interface *)>> m_serial;
  std::unique_ptr<Interface, std::function<void (Interface *)>> m_spi;
  std::unique_ptr<Timer, std::function<void (Timer *)>> m_sdioTimer;
  std::unique_ptr<Interface, std::function<void (Interface *)>> m_sdio;
  std::unique_ptr<Interface, std::function<void (Interface *)>> m_sdioWrapper;
  std::unique_ptr<FsHandle, std::function<void (FsHandle *)>> m_filesystem;
  SerialTerminal m_terminal;
  Initializer m_initializer;

  void bootstrap()
  {
    FsNode *parent;

    // Root nodes
    VfsNode *rootEntries[] = {
        new VfsDirectory{"bin", RealTimeClock::instance().getTime()},
        new VfsDirectory{"dev", RealTimeClock::instance().getTime()}
    };

    parent = static_cast<FsNode *>(fsHandleRoot(m_filesystem.get()));
    for (auto iter = std::begin(rootEntries); iter != std::end(rootEntries); ++iter)
    {
      const FsFieldDescriptor entryFields[] = {
          {&*iter, sizeof(*iter), static_cast<FsFieldType>(VfsNode::VFS_NODE_OBJECT)}
      };
      fsNodeCreate(parent, entryFields, ARRAY_SIZE(entryFields));
    }
    fsNodeFree(parent);

    // Device nodes
    VfsNode *deviceEntries[] = {
        new InterfaceNode<
            ParamDesc<IfDisplayParameter, IF_DISPLAY_ORIENTATION, uint8_t>,
            ParamDesc<IfDisplayParameter, IF_DISPLAY_RESOLUTION, DisplayResolution, uint16_t, uint16_t>,
            ParamDesc<IfDisplayParameter, IF_DISPLAY_WINDOW, DisplayWindow, uint16_t, uint16_t, uint16_t, uint16_t>
        >("display", m_display.get(), RealTimeClock::instance().getTime()),
        new InterfaceNode<
            ParamDesc<IfParameter, IF_RATE, uint32_t>
        >("sdio", m_sdio.get(), RealTimeClock::instance().getTime()),
        new InterfaceNode<
            ParamDesc<SerialParameter, IF_SERIAL_PARITY, uint8_t>,
            ParamDesc<IfParameter, IF_RATE, uint32_t>
        >("serial", m_serial.get(), RealTimeClock::instance().getTime())
    };

    parent = ShellHelpers::openNode(m_filesystem.get(), "/dev");
    for (auto iter = std::begin(deviceEntries); iter != std::end(deviceEntries); ++iter)
    {
      const FsFieldDescriptor entryFields[] = {
          {&*iter, sizeof(*iter), static_cast<FsFieldType>(VfsNode::VFS_NODE_OBJECT)}
      };
      fsNodeCreate(parent, entryFields, ARRAY_SIZE(entryFields));
    }
    fsNodeFree(parent);
  }

  static Interface *makeDisplayInterface(Interface *bus)
  {
    const S6D1121Config config = {
        bus,            // bus
        DISPLAY_CS,     // cs
        DISPLAY_RESET,  // reset
        DISPLAY_RS      // rs
    };
    return static_cast<Interface *>(init(S6D1121, &config));
  }

  static Interface *makeSdioInterface(Interface *spi, Timer *timer, PinNumber cs)
  {
    const SdioSpiConfig config{spi, timer, 0, cs};
    return static_cast<Interface *>(init(SdioSpi, &config));
  }

  static Interface *makeSdioWrapper(Interface *interface, PinNumber rx, PinNumber tx)
  {
    const InterfaceWrapper::Config config{interface, rx, tx};
    return static_cast<Interface *>(init(InterfaceWrapper, &config));
  }
};

const MemoryBusDmaConfig Application::s_displayBusConfig = {
    80,               // cycle
    s_displayBusPins, // pins
    0,                // priority

    // clock
    {
        PIN(1, 22), // leading
        PIN(1, 25), // trailing
        1,          // channel
        0,          // dma
        true        // inversion
    },

    // control
    {
        PIN(1, 26), // capture
        PIN(1, 28), // leading
        PIN(1, 29), // trailing
        0,          // channel
        1,          // dma
        false       // inversion
    }
};

const PinNumber Application::s_displayBusPins[] = {
    PIN(2, 0), PIN(2, 1), PIN(2, 2), PIN(2, 3),
    PIN(2, 4), PIN(2, 5), PIN(2, 6), PIN(2, 7),
    0
};

const GpTimerConfig Application::s_sdioTimerConfig = {
    100000,             // frequency
    GPTIMER_MATCH_AUTO, // event
    0,                  // priority
    2                   // channel
};

const SerialConfig Application::s_serialConfig = {
    115200,             // rate
    128,                // rxLength
    4096,               // txLength
    SERIAL_PARITY_NONE, // parity
    PIN(4, 29),         // rx
    PIN(4, 28),         // tx
    0,                  // priority
    3                   // channel
};

const SpiDmaConfig Application::s_spiConfig = {
    25000000,   // rate
    PIN(0, 17), // miso
    PIN(0, 18), // mosi
    PIN(0, 15), // sck
    0,          // channel
    3,          // mode
    {2, 3}      // dma
};

int main()
{
  enableClock();

  auto * const application = new Application();
  application->run();
  delete application;

  nvicResetCore();
  return 0; // Unreachable code
}
