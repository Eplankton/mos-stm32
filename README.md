# MOS-STM32

#### 介绍
```
 A_A       _
o'' )_____//
 `_/  MOS  )    STM32F4上的简单RTOS，适配Cortex-M系列
 (_(_/--(_/	 	MOS <=> Mini-RTOS

- MCU: STM32F429ZIT6 (256KB SRAM, 2MB FLASH)
- Board：Nucleo-144 F429ZI
```

#### 主要结构
```
mos/.
    | drivers/  	驱动抽象层(SPL/HAL)
    | task.hpp      任务创建、阻塞、挂起、终止
    | scheduler.hpp 调度器、上下文切换
    | globalres.hpp 全局变量
    | config.h      配置系统设定资源、宏

    kernel = task + scheduler + globalres

main.cpp    主函数入口
```
#### 使用
```C++
#include "mos/kernel.hpp"


namespace MOS::GlobalRes // 全局外设
{
	// 串口
	auto& uart = Driver::convert(USART3);

	// LED 红、绿、蓝
	Driver::LED_t leds[] = {
	    {GPIOB,  GPIO_Pin_14},
	    {GPIOB,  GPIO_Pin_0},
	    {GPIOB,  GPIO_Pin_7},
	};
}

namespace MOS::Bsp // 硬件配置
{
    using namespace Driver;

    void LED_Config()
	{
		using GlobalRes::leds;
        ...
		for (auto& led: leds) {
			led.init();
		}
	}

	void USART_Config()
	{
		using GlobalRes::uart;
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

namespace MOS::App // 用户任务函数
{
	void Task0(void* argv = nullptr)
	{
		while (true) {
			GlobalRes::leds[0].toggle();
			Task::print_name();
			Task::delay_ms(500);
		}
	}
}

void idle(void* argv = nullptr)
{
	using namespace MOS;

	Task::create(App::Task0, nullptr, 0, "T0");	// 创建用户任务
	Task::print_all_tasks(); // 打印所有任务列表

	while (true) {
        // ...
	}
}

int main(void) // 主函数入口
{
	using namespace MOS;

	Bsp::config(); // 硬件资源配置
	Task::create(idle, nullptr, 15, "idle"); // 创建 idle 任务
	Scheduler::launch(); // 调度开始，不再返回

	while (true) {
		// ...
	}
}
```

#### 烧录启动
```
 A_A       _
o'' )_____//
 `_/  MOS  )
 (_(_/--(_/ 

[MOS]: Hello :)  Build Time: xxx:xxx

Tid  Name   Priority    Status   MemUsage
=========================================
#0   idle      15       RUNNING    10%
#1   T0        0        READY       9%
=========================================
```

#### 版本
```
初始版本(0.0.1)，完成基本的调度器设计，计划完成以下部分：
1. 定时器，挂起队列，时间片轮转调度
2. 进程间通信 IPC，管道、消息队列
3. 进程同步 Sync，信号量、互斥锁
4. 移植简单的Shell
5. 可变页面大小，内存分配器
6. SPI 驱动开发，移植LVGL图形库
7. 移植到 ESP32-C3，RISC-V架构 
```