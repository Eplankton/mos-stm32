# MOS-STM32

#### Introduction

```
 A_A       _
o'' )_____//
 `_/  MOS  )    Mini RTOS on STM32F4, Cortex-M
 (_(_/--(_/     MOS <=> Mini-RTOS

- MCU:   STM32F429ZIT6 (256KB SRAM, 2MB FLASH)
- Board: Nucleo-144 F429ZI
```



#### Main Structure

```
mos/.
    | drivers/      Hardware Drivers (SPL/HAL/...)
    | task.hpp      Task create, yield, terminate, block
    | scheduler.hpp Scheduler, Context Switch
    | global.hpp    Kernel Global Resources
    | config.h      System Configuration

    kernel = task + scheduler + globalres
```


#### Example 

```C++
#include "mos/kernel.hpp"
#include "mos/shell.hpp"

namespace MOS::UserGlobal
{
    // Serial in and out
    auto& uart = Driver::convert(USART3);

    // LED red, green, blue
    Driver::LED_t leds[] = {
        {GPIOB,  GPIO_Pin_14},
        {GPIOB,  GPIO_Pin_0},
        {GPIOB,  GPIO_Pin_7},
    };
}

namespace MOS::Bsp
{
    using namespace Driver;

    void LED_Config()
    {
        using UserGlobal::leds;
        ...
        for (auto& led: leds) {
            led.init();
        }
    }

    void USART_Config()
    {
        using UserGlobal::uart;
        ...
        uart.init(...);
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
    using namespace UserGlobal;

    void Task0(void* argv)
    {
        while (true) {
            leds[0].toggle();
            Task::print_name();
            Task::delay_ms(500);
        }
    }
}

void idle(void* argv)
{
    using namespace MOS;

	Task::create(Shell::launch, nullptr, 1, "Shell");
    Task::create(App::Task0, nullptr, 0, "T0");
    Task::print_all_tasks();
	Task::change_priority(Task::current_task(), Macro::PRI_MIN);

    while (true) {
        // ...
    }
}

int main(void)
{
    using namespace MOS;
    Bsp::config();
    Task::create(idle, nullptr, Macro::PRI_MAX, "idle");
    Scheduler::launch(); // Begin Scheduling, never return

    while (true) {
        // ...
    }
}
```

#### Boot up

```
 A_A       _
o'' )_____//
 `_/  MOS  )  Build Time: xx:xx:xx
 (_(_/--(_/

Tid  Name   Priority   Status    PageUsage
-------------------------------------------
#0   idle     15       RUNNING      10%
#1   T0       0        READY         9%
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
2. Policy::{PreemptivePriority}, allows tasks of the same priority to be scheduled in round-robin
3. Task::terminate is called implicitly when the task exits
4. Shell::{Command, CmdCall, launch}

To do:
1. Mutex_t with priority inheritance mechanism
2. Inter-process communication: pipes, message queues, etc
3. Simple dynamic memory allocator
```
