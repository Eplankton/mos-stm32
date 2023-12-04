# MOS-STM32

#### Introduction
```
 A_A       _
o'' )_____//
 `_/  MOS  )    Mini RTOS on STM32F4, Cortex-M
 (_(_/--(_/     MOS <=> Mini-RTOS

- Board: Nucleo-144 F429ZI
- MCU:   STM32F429ZIT6 (256KB SRAM, 2MB FLASH)
```

#### Structure
```
src
├── mos_config             System Configuration
├── main.cpp               Entry point
│
├── drivers                Hardware Drivers(SPL/HAL/...)
│   ├── device             Other hardware(LED, LCD, etc.)
│   └── stm32f4xx          STM32F4xx on-chip periphs
│
└── mos
    ├── arch               Arch-related code
    │   └── cpu.hpp
    │
    └── kernel             Arch-independent code
        ├── global.hpp     Kernel global
        ├── scheduler.hpp  Scheduler, Policy
        ├── sync.hpp       Sync primitive
        └── task.hpp       Task create, yield, terminate, block...
```

#### Example 
![demo1 00_00_00-00_00_30](https://github.com/Eplankton/mos-stm32/assets/86543401/65e36ea0-d178-4da6-8f9a-9f1551c59dfc)
![demo2 00_00_00-00_00_30](https://github.com/Eplankton/mos-stm32/assets/86543401/bdd2c288-4528-45d5-b07c-03fe1c66bb34)
![board](https://github.com/Eplankton/mos-stm32/assets/86543401/93cf5645-2d72-4c52-bad3-aec935a4510f)
![demo3](https://github.com/Eplankton/mos-stm32/assets/86543401/45069534-88db-448b-8452-b930ad82395e)

```C++
// MOS Kernel & Shell
#include "mos/kernel/kernel.hpp"
#include "mos/shell.hpp"

// STM32F4xx HAL
#include "drivers/stm32f4xx/hal.hpp"

// Devices
#include "drivers/device/led.hpp"

namespace MOS::UserGlobal
{
    using namespace HAL::STM32F4xx;
    using namespace Driver;
    
    // Serial in and out
    auto& uart = STM32F4xx::convert(USART3);

    // LED red, green, blue
    Driver::LED_t leds[] = {...};
}

namespace MOS::Bsp
{
    using namespace Driver;
    using namespace UserGlobal;

    void LED_Config()
    {
        ...
        for (auto& led: leds) {
            led.init();
        }
    }

    void USART_Config()
    {
        ...
        uart.init(9600-8-1-N);
    }

    void config()
    {
        ...
        LED_Config();
        USART_Config();
        ...
    }
}

namespace MOS::App // User tasks
{
    using UserGlobal::leds;

    void Task0(void* argv)
    {
        while (true) {
            leds[0].toggle();
            Task::delay(500);
        }
    }
}

int main(void)
{
    using namespace MOS;

    Bsp::config(); // Init Hardware
    Task::create(App::Task0, nullptr, 1, "T0"); // Create user task
    Scheduler::launch(); // Begin Scheduling, never return

    while (true) {
        // ...
    }
}
```

#### Boot up

```
 A_A       _
o'' )_____//  Version  @ x.x.x
 `_/  MOS  )  Platform @ xxx, xxx
 (_(_/--(_/   Build    @ xx:xx:xx

Tid  Name   Priority   Status    PageUsage
-------------------------------------------
#0   idle     15       READY          10%
#1   T0       0        RUNNING         9%
-------------------------------------------
```

#### Version

```
The initial version (0.0.1) with basic scheduler, to do:
1. Timers, RoundRobin
2. Inter-process communication(IPC), pipes, message queues
3. Process synchronization Sync, semaphores, mutex lock
4. Porting simple shells
5. Variable page size, memory allocator
6. SPI driver, transplant LVGL graphics library
7. Port to ESP32-C3, RISC-V
```

```
Version 0.0.2:
1. Sync::{Semaphore_t, Lock_t}
2. Scheduler::Policy::{PreemptivePriority}, under same priority -> {RoundRobin}
3. Task::terminate() implicitly be called when task exits
4. Shell::{Command, CmdCall, launch}
5. os_ticks and Task::delay() as blocking delay
6. Driver::{SPI_t, ST7735S}
7. Reorganize the struct of project into {kernel, arch, driver}
8. Support GuiLite: https://github.com/idea4good/GuiLite
9. Support GCC and STM32 HAL

To do:
1. Mutex_t with priority inheritance mechanism
2. IPC: pipes, message queues, etc.
3. Simple dynamic memory allocator
4. Timers
```