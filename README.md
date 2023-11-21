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
| mos/.
|     | drivers/.               Hardware Drivers(SPL/HAL/...)
|     | 
|     | arch/. 
|     |      | cpu.hpp          Arch-related code
|     | 
|     | kernel/.                Arch-independent code
|              | global.hpp     Kernel global
|              | task.hpp       Task create, yield, terminate, block...
|              | scheduler.hpp  Scheduler
|              | sync.hpp       Synchronization primitive
|
| main.cpp                      Entry point
| config.h                      System Configuration
```

#### Example 
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
2. Policy::{PreemptivePriority}, for same priority -> {RoundRobin}
3. Task::terminate() implicitly be called when task exits
4. Shell::{Command, CmdCall, launch}
5. os_ticks and Task::delay(ticks)
6. Driver::{SPI_t, ST7735S}
7. Reorganize the struct of project to {kernel, arch, driver}

To do:
1. Mutex_t with priority inheritance mechanism
2. IPC: pipes, message queues, etc.
3. Simple dynamic memory allocator
4. GUILite/LVGL
5. Timers
```
