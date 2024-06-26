## MOS-STM32 🧀

### Introduction 🚀
[English](https://github.com/Eplankton/mos-stm32/blob/master/README.md) | [中文](https://gitee.com/Eplankton/mos-stm32/blob/master/README.md)
```
 A_A       _
o'' )_____//    [MOS-STM32]
 `_/  MOS  )    Mini RTOS on Cortex-M
 (_(_/--(_/     MOS <=> Mini-RTOS

- Board: NUCLEO-144 F429ZI
- MCU:   STM32F429ZIT6 (256KB SRAM, 2MB FLASH)
```

### Repository 🌏
[GitHub](https://github.com/Eplankton/mos-stm32) | [Gitee](https://gitee.com/Eplankton/mos-stm32/)

### Architecture 🔍
[USR/src](https://github.com/Eplankton/mos-stm32/tree/master/USR/src)
<img src="Pic/mos-arch.svg">
```C++
.
├── vendor              // Hardware Abstraction Layer(SPL/HAL/LL/...)
└── src
    ├── drivers         // Hardware Driver Portable Interface
    │   ├── stm32f4xx   // STM32F4xx On-Chip Peripherals(USART, I2C, SPI, ...)
    │   └── device      // Other components(LED, LCD, ...)
    │
    ├── mos
    │   ├── config.h             // System Configuration
    │   ├── arch                 // Arch-related
    │   │   └── cpu.hpp          // asm for init/context_switch
    │   │
    │   ├── kernel               // Kernel(Arch-independent)
    │   │   ├── macro.hpp        // Kernel Constant Macros
    │   │   ├── type.hpp         // Basic Types
    │   │   ├── concepts.hpp     // Type Constraints(Optional)
    │   │   ├── data_type.hpp    // Basic Data Structures
    │   │   ├── alloc.hpp        // Static/Dynamic Allocator
    │   │   ├── global.hpp       // Kernel Globals
    │   │   ├── printf.h/.c      // Thread-safe printf (by mpaland)
    │   │   ├── task.hpp         // Task control
    │   │   ├── sync.hpp         // Sync primitives
    │   │   ├── scheduler.hpp    // Scheduler and Policy
    │   │   ├── ipc.hpp          // Inter-Process Communication
    │   │   └── utils.hpp        // Utils
    │   │
    │   ├── kernel.hpp           // Import Kernel Modules
    │   └── shell.hpp            // Simple Shell
    │
    ├── user
    │   ├── gui                  // GUI-related
    │   │   ├── GuiLite.h        // GuiLite Framework
    │   │   └── UICode.cpp       // User Interface
    │   │
    │   ├── global.hpp           // User Globals
    │   ├── bsp.hpp              // Board Support Package
    │   ├── app.hpp              // User Applications
    │   └── test.hpp             // Test
    │
    ├── main.cpp                 // Entry main()
    └── stm32f4xx_it.cpp         // Interrput SubRoutine(Partly)
```

### Example 🍎
`Shell`
![shell_demo](Pic/shell.gif)

`MutexTest`
![mutex_test](Pic/mutex.gif)

`LCD Driver & GUI Demo`<br>
<img src="Pic/cat.gif" width="21%"> <img src="Pic/mac.gif" width="20.35%"> <img src="Pic/face.gif" width="20.35%">
<img src="Pic/board.gif" width="39.1%"> <img src="Pic/guilite.gif" width="34.5%">

`Concurrent Task Period & Time Sequence`<br>
<img src="Pic/T0-T1.png" width="80%">
<img src="Pic/tids.png" width="65%">


`Async Executor`
<img src="Pic/async.png">

```C++
// MOS Kernel & Shell
#include "mos/kernel.hpp"
#include "mos/shell.hpp"

// HAL and Device 
#include "drivers/stm32f4xx/hal.hpp"
#include "drivers/device/led.hpp"
```
```C++
namespace MOS::User::Global
{
    using namespace HAL::STM32F4xx;
    using namespace Driver::Device;
    using namespace DataType;

    // Shell I/O UART and Buffer
    auto& stdio = STM32F4xx::convert(USARTx);
    DataType::SyncRxBuf_t<16> io_buf;

    // LED red, green, blue
    Device::LED_t leds[] = {...};
}
```
```C++
namespace MOS::User::BSP
{
    using namespace Driver;
    using namespace Global;

    void LED_Config()
    {
        for (auto& led: leds) {
            led.init();
        }
    }

    void USART_Config()
    {
        // Simplified
        stdio.init(9600-8-1-N)
             .rx_config(PXa)  // RX -> PXa
             .tx_config(PYb)  // TX -> PYb
             .it_enable(RXNE) // Enable RXNE interrupt
             .enable();
    }
    ...
}
```
```C++
namespace MOS::User::App
{
    Sync::Barrier_t bar {2};

    void LED_1(Device::LED_t leds[])
    {
        bar.wait();
        for (auto _: Range(0, 20)) {
           leds[1].toggle(); // green
           Task::delay(250_ms);
        }
        kprintf("L1 exits...\n");
    }

    void LED_0(Device::LED_t leds[])
    {
        Task::create(
            LED_1, 
            leds, 
            Task::current()->get_pri(),
            "L1"
        );
        bar.wait();
        while (true) {
            leds[0].toggle(); // red
            Task::delay(500_ms);
        }
    }
}
```
```C++
int main()
{
    using namespace MOS;
    using namespace Kernel;
    using namespace User;
    using namespace User::Global;

    BSP::config(); // Init hardware and clocks

    Task::create( // Create Calendar with RTC
        App::Calendar, nullptr, 0, "Calendar"
    );

    Task::create( // Create Shell with io_buf
        Shell::launch, &io_buf, 1, "Shell"
    );

    /* User Tasks */
    Task::create(App::LED_0, &leds, 2, "L0");
    ...

    /* Test examples */
    Test::MutexTest();
    Test::MsgQueueTest();
    ...
    
    // Start scheduling, never return
    Scheduler::launch();
}
```

### Boot Up ⚡
```
 A_A       _   Version @ x.x.x(...)
o'' )_____//   Build   @ TIME, DATE
 `_/  MOS  )   Chip    @ MCU, ARCH
 (_(_/--(_/    2023-2024 Copyright by Eplankton

 Tid   Name   Priority   Status   Stack%
-----------------------------------------
 #0    idle      15      READY       10%
 #1    Shell      1      BLOCKED     21%
 #2    L0         2      RUNNING      9%
-----------------------------------------
```

### Version 🧾
---
📦 `v0.1`

✅ Done
> - Basic Scheduler and Task control, memory management

📌 Plan
> - Timers, `RoundRobin`
> - Inter-process communication `IPC`, pipes, message queues
> - Sync, semaphore, mutex, lock
> - Porting simple shells
> - Mutable page size, memory allocator
> - `SPI` driver and `LVGL` library
> - Port to other platform like `ESP32-C3(RISC-V)`
---
📦 `v0.2`

✅ Done
> - `Sync::{Sema_t, Lock_t, Mutex_t<T>, CondVar_t, Barrier_t}`, where `Mutex_t` adopts Priority Ceiling Protocol
> - `Scheduler::Policy::PreemptPri`, under same priority -> `RoundRobin`
> - `Task::terminate` will be implicitly called when task exits
> - `Shell::{Command, CmdCall, launch}`
> - `HAL::STM32F4xx::SPI_t` and `Driver::ST7735S_t`, support `GuiLite`
> - `Kernel::Global::os_ticks` and `Task::delay` for blocking delay
> - Refactor the project into `{kernel, arch, drivers}`
> - Support `GCC` and `STM32CubeMX HAL`
> - `HAL::STM32F4xx::RTC_t`, `CmdCall::date_cmd` and `App::Calendar`
> - `idle` uses `Kernel::Global::zombie_list` to recycle inactive pages
> - Three basic page allocator policies, `Page_t::Policy::{POOL, DYNAMIC, STATIC}`
---
📦 `v0.3`

✅ Done
> - `Tids` from `BitMap_t`
> - (Experimental) `Task::Async::{Future_t, async/await}`
> - `IPC::MsgQueue_t`, Message Queue
> - `Task::create` allows generic `fn` signature as `void fn(auto argv)` with type check
> - Add `ESP32C3` as `WiFi` Module
> - (**Experimental**) Atomic Type in `<stdatomic.h>`
> - (**Experimental**) `Utils::IntrGuard_t`, Nested Interrupt Lock Guard
> - Add `Driver::Device::SD_t` driver with `SPI` mode
> - Add `FatFs` as File System
> - Add `Shell::usr_cmds`，User Register Service

📌 Plan
> - `IPC::pipe/channel`
> - Soft/Hardware Timers
> - (**Experimental**) Async Stackless Coroutine `Async::{Future_t, async/await}`
> - (**Experimental**) Basic Formal Verification on `Scheduler`
> - `DMA_t` Driver
> - More real-time scheduling algorithms
> - `FPU` support
> - `Result<T, E>, Option<T>`
> - Add `POSIX` support
> - Performance Benchmark
---

### References 🛸
- [How to build a Real-Time Operating System(RTOS)](https://medium.com/@dheeptuck/building-a-real-time-operating-system-rtos-ground-up-a70640c64e93)
- [PeriodicScheduler_Semaphore](https://github.com/Dungyichao/PeriodicScheduler_Semaphore)
- [STM32F4-LCD_ST7735s](https://github.com/Dungyichao/STM32F4-LCD_ST7735s)
- [A printf/sprintf Implementation for Embedded Systems](https://github.com/mpaland/printf)
- [GuiLite](https://github.com/idea4good/GuiLite)
- [STMViewer](https://github.com/klonyyy/STMViewer)
- [FatFs](http://elm-chan.org/fsw/ff)

```
Wake up, Neo...
The Matrix has you...
Follow the white rabbit.
Knock, knock, Neo.
```