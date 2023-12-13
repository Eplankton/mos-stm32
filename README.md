# MOS-STM32 🐱‍🚀

### Introduction 🚀
```
 A_A       _
o'' )_____//    [MOS-STM32]
 `_/  MOS  )    Mini RTOS on STM32F4, Cortex-M
 (_(_/--(_/     MOS <=> Mini-RTOS

- Board: Nucleo-144 F429ZI
- MCU:   STM32F429ZIT6 (256KB SRAM, 2MB FLASH)
```

### Repository 🌏
[GitHub](https://github.com/Eplankton/mos-stm32) | [Gitee](https://gitee.com/Eplankton/mos-stm32/)

### Structure 👾
[Here's USR/src](https://github.com/Eplankton/mos-stm32/tree/master/USR/src)
```    
src
├── drivers                  Hardware Drivers(SPL/HAL/...)
│   ├── stm32f4xx            STM32F4xx on-chip periphs
│   └── device               Other hardware(LED, LCD, etc.)
│
├── mos
│   ├── config.h             System Configuration
│   ├── arch                 Arch-related code
│   │   └── cpu.hpp          ASM of ContextSwitch
│   │
│   ├── kernel               Kernel(arch-independent)
│   │   ├── macro.hpp        Configured Macros
│   │   ├── type.hpp         Basic Types
│   │   ├── concepts.hpp     C++20 Concepts
│   │   ├── data_type.hpp    Kernel Data Structures
│   │   ├── alloc.hpp        Static/Dynamic Allocator
│   │   ├── global.hpp       Kernel Globals
│   │   ├── printf.c         printf implementation
│   │   ├── task.hpp         Task create, yield, terminate, block...
│   │   ├── sync.hpp         Sync primitives
│   │   ├── scheduler.hpp    Scheduler and Policy
│   │   └── util.hpp         Utils
│   │
│   ├── kernel.hpp           Import Kernel Modules
│   └── shell.hpp            Simple Shell
│
├── user
│   ├── gui                  GUI-related
│   │   ├── GuiLite.h        GuiLite Framework
│   │   └── UICode.cpp       Animation
│   │
│   ├── global.hpp           User Globals
│   ├── bsp.hpp              Board Support Package
│   ├── app.hpp              Applications
│   └── test.hpp             Test
│
├── main.cpp                 Entry main()
└── stm32f4xx_it.cpp         Interrput SubRoutine(partly)
```

### Example 🍎
![demo1 00_00_00-00_00_30](https://github.com/Eplankton/mos-stm32/assets/86543401/65e36ea0-d178-4da6-8f9a-9f1551c59dfc)
![demo2 00_00_00-00_00_30](https://github.com/Eplankton/mos-stm32/assets/86543401/bdd2c288-4528-45d5-b07c-03fe1c66bb34)
<img src="https://github.com/Eplankton/mos-stm32/assets/86543401/93cf5645-2d72-4c52-bad3-aec935a4510f" width="45%"> <img src="https://github.com/Eplankton/mos-stm32/assets/86543401/45069534-88db-448b-8452-b930ad82395e" width="45%">

```C++
// MOS Kernel & Shell
#include "mos/kernel.hpp"
#include "mos/shell.hpp"
#include "drivers/stm32f4xx/hal.hpp" // STM32F4xx HAL
#include "drivers/device/led.hpp" // Devices

namespace MOS::UserGlobal
{
    using namespace HAL::STM32F4xx;
    using namespace Driver;
    
    // Serial TX/RX
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
        for (auto& led: leds) {
            led.init();
        }
    }

    void USART_Config()
    {
        uart.init(9600-8-1-N);
    }

    void config()
    {
        LED_Config();
        USART_Config();
        SysTick_Config();
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
        // Never comes here...
    }
}
```

### Boot up ⚡
```
 A_A       _
o'' )_____//  Version  @ x.x.x
 `_/  MOS  )  Platform @ xxx, xxx
 (_(_/--(_/   Build    @ xx:xx:xx

Tid  Name   Priority   Status   StackUsage
------------------------------------------
#0   idle     15       READY       10%
#1   T0       0        RUNNING      9%
------------------------------------------
```

### Version 📝
```
📦The initial version (0.0.1) with basic scheduler
📌To do:
1. Timers, RoundRobin
2. Inter-process communication(IPC), pipes, message queues
3. Sync, semaphore, mutex, lock
4. Porting simple shells
5. Mutable page size, memory allocator
6. SPI driver, LVGL library
7. Port to ESP32-C3, RISC-V
```

```
📦Version 0.0.2:
1. Sync::{Semaphore_t, Lock_t}
2. Scheduler::Policy::{PreemptivePriority}, under same priority -> {RoundRobin}
3. Task::terminate() implicitly be called when task exits
4. Shell::{Command, CmdCall, launch}
5. os_ticks and Task::delay() as blocking delay
6. Driver::{SPI_t, ST7735S}
7. Reorganize the struct of project into {kernel, arch, driver}
8. Support GuiLite: https://github.com/idea4good/GuiLite
9. Support GCC and STM32 HAL

📌To do:
1. Mutex_t with priority inheritance mechanism
2. IPC: pipes, message queues, etc.
3. Simple dynamic memory allocator
4. Hardware Timers
5. BitMap on pages for faster allocation
6. Basic formal verification on Scheduler
```

### References 🛸
1. [How to build a Real-Time Operating System(RTOS)](https://medium.com/@dheeptuck/building-a-real-time-operating-system-rtos-ground-up-a70640c64e93)
2. [PeriodicScheduler_Semaphore](https://github.com/Dungyichao/PeriodicScheduler_Semaphore)
3. [STM32F4-LCD_ST7735s](https://github.com/Dungyichao/STM32F4-LCD_ST7735s)
4. [A printf/sprintf Implementation for Embedded Systems](https://github.com/mpaland/printf)
5. [GuiLite](https://github.com/idea4good/GuiLite)