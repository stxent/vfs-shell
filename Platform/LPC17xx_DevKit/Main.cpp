/*
 * Main.cpp
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "CardBuilder.hpp"
#include "InterfaceWrapper.hpp"
#include "RealTimeClock.hpp"
#include "Scripts/CpuFrequencyScript.hpp"
#include "Scripts/MakeDacScript.hpp"
#include "Scripts/MakePinScript.hpp"
#include "Scripts/RtcUtilScript.hpp"
#include "Scripts/ShutdownScript.hpp"

#include "Shell/Initializer.hpp"
#include "Shell/Interfaces/InterfaceNode.hpp"
#include "Shell/Scripts/ChangeDirectoryScript.hpp"
#include "Shell/Scripts/ChecksumCrc32Script.hpp"
#include "Shell/Scripts/CopyNodeScript.hpp"
#include "Shell/Scripts/DateScript.hpp"
#include "Shell/Scripts/DirectDataScript.hpp"
#include "Shell/Scripts/DisplayTestScript.hpp"
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

#include <halm/core/cortex/nvic.h>
#include <halm/generic/sdio_spi.h>
#include <halm/pin.h>
#include <halm/platform/lpc/bod.h>
#include <halm/platform/lpc/gptimer.h>
#include <halm/platform/lpc/i2c.h>
#include <halm/platform/lpc/lpc17xx/clocking.h>
#include <halm/platform/lpc/serial.h>
#include <halm/platform/lpc/spi_dma.h>
#include <halm/pm.h>
#include <dpm/displays/display.h>
#include <dpm/displays/ili9325.h>
#include <dpm/displays/s6d1121.h>
#include <dpm/platform/lpc/memory_bus_dma.h>
#include <xcore/fs/utils.h>

#include <cassert>

static void enableClock()
{
  static const ExternalOscConfig extOscConfig = {12000000, false};
  static const PllConfig sysPllConfig = {3, 25, CLOCK_EXTERNAL};
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
  static constexpr PinNumber DISPLAY_BL{PIN(1, 26)};
  static constexpr PinNumber DISPLAY_CS{PIN(1, 14)};
  static constexpr PinNumber DISPLAY_RESET{PIN(1, 4)};
  static constexpr PinNumber DISPLAY_RS{PIN(1, 0)};
  static constexpr PinNumber DISPLAY_RW{PIN(1, 1)};

  static constexpr PinNumber LED_B{PIN(1, 8)};
  static constexpr PinNumber LED_G{PIN(1, 9)};
  static constexpr PinNumber LED_R{PIN(1, 10)};
  static constexpr PinNumber SDIO_CS_PIN{PIN(0, 22)};

  static const BodConfig s_bodConfig;
  static const MemoryBusDmaConfig s_displayBusConfig;
  static const PinNumber s_displayBusPins[];
  static const I2CConfig s_i2cConfig;
  static const GpTimerConfig s_sdioTimerConfig;
  static const SerialConfig s_serialConfig;
  static const SpiDmaConfig s_spiConfig;

  void bootstrap();
  static void interfaceDeleter(Interface *pointer);
  static Interface *makeDisplayInterface(Interface *bus);
  static Interface *makeSdioInterface(Interface *spi, Timer *timer, PinNumber cs);
  static Interface *makeSdioWrapper(Interface *interface, PinNumber rx, PinNumber tx);

public:
  Application() :
    m_bod{static_cast<Interrupt *>(init(Bod, &s_bodConfig)), [](Interrupt *pointer){ deinit(pointer); }},
    m_displayBus{static_cast<Interface *>(init(MemoryBusDma, &s_displayBusConfig)), interfaceDeleter},
    m_display{makeDisplayInterface(m_displayBus.get()), interfaceDeleter},
    m_i2c{static_cast<Interface *>(init(I2C, &s_i2cConfig)), interfaceDeleter},
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
    m_initializer.attach<DisplayTestScript<BUFFER_SIZE>>();
    m_initializer.attach<EchoScript>();
    m_initializer.attach<ExitScript>();
    m_initializer.attach<GetEnvScript>();
    m_initializer.attach<HelpScript>();
    m_initializer.attach<ListEnvScript>();
    m_initializer.attach<ListNodesScript>();
    m_initializer.attach<MakeDacScript>();
    m_initializer.attach<MakeDirectoryScript>();
    m_initializer.attach<MakePinScript>();
    m_initializer.attach<MountScript<CardBuilder>>();
    m_initializer.attach<PrintHexDataScript<BUFFER_SIZE>>();
    m_initializer.attach<PrintRawDataScript<BUFFER_SIZE>>();
    m_initializer.attach<RemoveNodesScript>();
    m_initializer.attach<RtcUtilScript<RealTimeClock>>(&RealTimeClock::instance());
    m_initializer.attach<SetEnvScript>();
    m_initializer.attach<Shell>();
    m_initializer.attach<ShutdownScript>();
    m_initializer.attach<TimeScript>();

    return m_initializer.run() == E_OK ? EXIT_SUCCESS : EXIT_FAILURE;
  }

private:
  static constexpr size_t BUFFER_SIZE{2048};

  std::unique_ptr<Interrupt, std::function<void (Interrupt *)>> m_bod;
  std::unique_ptr<Interface, std::function<void (Interface *)>> m_displayBus;
  std::unique_ptr<Interface, std::function<void (Interface *)>> m_display;
  std::unique_ptr<Interface, std::function<void (Interface *)>> m_i2c;
  std::unique_ptr<Interface, std::function<void (Interface *)>> m_serial;
  std::unique_ptr<Interface, std::function<void (Interface *)>> m_spi;
  std::unique_ptr<Timer, std::function<void (Timer *)>> m_sdioTimer;
  std::unique_ptr<Interface, std::function<void (Interface *)>> m_sdio;
  std::unique_ptr<Interface, std::function<void (Interface *)>> m_sdioWrapper;
  std::unique_ptr<FsHandle, std::function<void (FsHandle *)>> m_filesystem;
  SerialTerminal m_terminal;
  Initializer m_initializer;
};

const BodConfig Application::s_bodConfig = {
    BOD_EVENT_2V2,      // eventLevel
    BOD_RESET_DISABLED, // resetLevel
    0                   // priority
};

const MemoryBusDmaConfig Application::s_displayBusConfig = {
    s_displayBusPins, // pins
    32768,            // size
    80,               // cycle
    0,                // priority

    // clock
    {
        PIN(1, 28),   // leading
        PIN(1, 29),   // trailing
        0,            // channel
        1,            // dma
        false,        // inversion
        true          // swap
    },

    // control
    {
        PIN(1, 19),   // capture
        PIN(1, 22),   // leading
        PIN(1, 25),   // trailing
        1,            // channel
        0,            // dma
        false         // inversion
    }
};

const PinNumber Application::s_displayBusPins[] = {
    PIN(2, 0), PIN(2, 1), PIN(2, 2), PIN(2, 3),
    PIN(2, 4), PIN(2, 5), PIN(2, 6), PIN(2, 7),
    0
};

const I2CConfig Application::s_i2cConfig = {
    400000,     // rate
    PIN(0, 11), // scl
    PIN(0, 10), // sda
    0,          // priority
    2           // channel
};

const GpTimerConfig Application::s_sdioTimerConfig = {
    100000,             // frequency
    GPTIMER_MATCH_AUTO, // event
    0,                  // priority
    2                   // channel
};

const SerialConfig Application::s_serialConfig = {
    128,                // rxLength
    4096,               // txLength
    115200,             // rate
    SERIAL_PARITY_NONE, // parity
    PIN(0, 16),         // rx
    PIN(0, 15),         // tx
    0,                  // priority
    1                   // channel
};

const SpiDmaConfig Application::s_spiConfig = {
    10000000,   // rate
    PIN(0, 17), // miso
    PIN(0, 18), // mosi
    PIN(1, 20), // sck
    0,          // priority
    0,          // channel
    3,          // mode
    {2, 3}      // dma
};

void Application::bootstrap()
{
  VfsNode *node;

  // Root nodes

  node = new VfsDirectory{RealTimeClock::instance().getTime()};
  ShellHelpers::injectNode(m_filesystem.get(), node, "/bin");

  node = new VfsDirectory{RealTimeClock::instance().getTime()};
  ShellHelpers::injectNode(m_filesystem.get(), node, "/dev");

  // Device nodes

  node = new InterfaceNode<
          ParamDesc<DisplayParameter, IF_DISPLAY_ORIENTATION, DisplayOrientation>,
          ParamDesc<DisplayParameter, IF_DISPLAY_RESOLUTION, DisplayResolution, uint16_t, uint16_t>,
          ParamDesc<DisplayParameter, IF_DISPLAY_WINDOW, DisplayWindow, uint16_t, uint16_t, uint16_t, uint16_t>
      >{m_display.get(), RealTimeClock::instance().getTime()};
  ShellHelpers::injectNode(m_filesystem.get(), node, "/dev/display");

  node = new InterfaceNode<
          ParamDesc<IfParameter, IF_RATE, uint32_t>
      >{m_sdio.get(), RealTimeClock::instance().getTime()};
  ShellHelpers::injectNode(m_filesystem.get(), node, "/dev/sdio");

  node = new InterfaceNode<
          ParamDesc<SerialParameter, IF_SERIAL_PARITY, uint8_t>,
          ParamDesc<IfParameter, IF_RATE, uint32_t>
      >{m_serial.get(), RealTimeClock::instance().getTime()};
  ShellHelpers::injectNode(m_filesystem.get(), node, "/dev/serial");
}

void Application::interfaceDeleter(Interface *pointer)
{
  deinit(pointer);
}

Interface *Application::makeDisplayInterface(Interface *bus)
{
  pinOutput(pinInit(DISPLAY_RW), false);
  pinOutput(pinInit(DISPLAY_BL), true);

  const ILI9325Config config = {
      bus,            // bus
      DISPLAY_CS,     // cs
      DISPLAY_RESET,  // reset
      DISPLAY_RS      // rs
  };
  auto display = static_cast<Interface *>(init(ILI9325, &config));

  if (display != nullptr)
  {
    const uint32_t orientation = DISPLAY_ORIENTATION_NORMAL;
    ifSetParam(display, IF_DISPLAY_ORIENTATION, &orientation);

    struct DisplayResolution resolution;
    ifGetParam(display, IF_DISPLAY_RESOLUTION, &resolution);

    const struct DisplayWindow window = {
        0,
        0,
        static_cast<uint16_t>(resolution.width - 1),
        static_cast<uint16_t>(resolution.height - 1)
    };
    ifSetParam(display, IF_DISPLAY_WINDOW, &window);
  }

  return display;
}

Interface *Application::makeSdioInterface(Interface *spi, Timer *timer, PinNumber cs)
{
  const SdioSpiConfig config{spi, timer, nullptr, 0, cs};
  return static_cast<Interface *>(init(SdioSpi, &config));
}

Interface *Application::makeSdioWrapper(Interface *interface, PinNumber rx, PinNumber tx)
{
  const InterfaceWrapper::Config config{interface, rx, tx};
  return static_cast<Interface *>(init(InterfaceWrapper, &config));
}

int main()
{
  enableClock();

  auto * const application = new Application();
  application->run();
  delete application;

  nvicResetCore();
  return 0; // Unreachable code
}
