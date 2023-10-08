# MOS-STM32

#### 介绍
```
 A_A       _
o'' )_____//
 `_/  MOS  )    MOS <=> Mini-RTOS
 (_(_/--(_/

- STM32F4上的简单RTOS，适配Cortex-M系列
- 测试MCU: STM32F429ZIT6 (256KB SRAM, 2MB FLASH)
```

#### 主要结构
```
mos/.
    | drivers/  硬件抽象层
    | task.hpp      任务创建、阻塞、挂起
    | scheduler.hpp 调度器与上下文切换
    | globalres.hpp 全局变量
    | config.h      配置系统资源和宏

    kernel = task + scheduler + globalres

main.cpp    主函数入口
```
#### 使用
```C++
#include "mos/kernel.hpp"

// 将全局外设声明在这里
namespace MOS::GlobalRes
{
	using namespace Driver;

	// Serial input and output
	auto& uart = Driver::convert(USART3);

	// LED red, green, blue
	Driver::LED_t leds[] = {
	    {GPIOB, GPIO_Pin_14},
	    {GPIOB,  GPIO_Pin_0},
	    {GPIOB,  GPIO_Pin_7},
	};
}

// 将硬件配置函数放在这里
namespace MOS::Bsp 
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
		LED_Config();
		USART_Config();
	}
}

// 将用户任务函数放在这里
namespace MOS::App
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

	// 创建用户任务
	Task::create(App::Task0, nullptr, 0, "T0");

	// 打印所有任务信息
	Task::print_all_tasks();

	while (true) {
        // idle
	}
}

// 主函数入口
int main(void)
{
	using namespace MOS;

	Bsp::config();                           // 硬件资源配置
	Task::create(idle, nullptr, 15, "idle"); // 创建 idle
	Scheduler::launch();                     // 启动调度器，不再返回

	while (true) {
		// 不会进入这里
	}
}
```

#### 启动现象
```
 A_A       _
o'' )_____//
 `_/  MOS  )
 (_(_/--(_/ 

[MOS]: Hello :)  Build Time: 16:27:48, Oct  8 2023

Tid  Name   Priority   Status   MemUsage
========================================
#0   idle     15       RUNNING    10%
#1   T0       0        READY       9%
========================================
```

#### 版本
```
初始版本(0.0.1)，完成基本的调度器设计，计划完成以下三个部分：
1. 进程间通信 IPC，管道、消息队列等
2. 同步 Sync，信号量、互斥锁等
3. 移植简单的shell，能够使用命令启动任务
```