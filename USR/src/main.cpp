#include "main.h"
#include "mos.hpp"

// Put all global resource here
namespace MOS::GlobalRes
{
	// Serial input && output
	auto& uart = convert(USART3);

	// LED red, green, blue
	LED_t leds[] = {
	        {GPIOB, GPIO_Pin_14},
	        {GPIOB,  GPIO_Pin_0},
	        {GPIOB,  GPIO_Pin_7},
	};
}

namespace MOS::Task
{
	inline void print_name()
	{
		DISABLE_IRQ();
		uart.println(curTCB->name);
		ENABLE_IRQ();
	}

	inline void delay(const uint32_t n)
	{
		nuts::delay(n);
	}
}

static void NVIC_Config()
{
	NVIC_t::group_config(NVIC_PriorityGroup_2);
}

static void LED_Config()
{
	using MOS::GlobalRes::leds;

	RCC_t::AHB1::clock_cmd(RCC_AHB1Periph_GPIOB, ENABLE);
	for (auto& led: leds) {
		led.init();
	}
}

static void USART_Config()
{
	using MOS::GlobalRes::uart;

	RCC_t::AHB1::clock_cmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_t::APB1::clock_cmd(RCC_APB1Periph_USART3, ENABLE);

	uart.init(9600, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No)
	        .rx_config(GPIOD, GPIO_t::get_pin_src(9), GPIO_AF_USART3)
	        .tx_config(GPIOD, GPIO_t::get_pin_src(8), GPIO_AF_USART3)
	        .enable();
}

static void Welcome()
{
	using MOS::GlobalRes::uart;
	uart.send_string("[MOS]: Hello :)\n");
}

static void Resource_Config()
{
	NVIC_Config();
	USART_Config();
	LED_Config();
	Welcome();
}

void idle(void* argv = nullptr)
{
	while (true) {
		// Idle does nothing but loop...
		MOS::Task::delay(1000);
		MOS::Task::print_name();
	}
}

void Task0(void* argv = nullptr)
{
	for (uint8_t i = 0; i < 10; i++) {
		MOS::Task::delay(500);
		MOS::GlobalRes::leds[0].toggle();
		MOS::Task::print_name();
	}
	MOS::Task::terminate();
}

void Task1(void* argv = nullptr)
{
	for (uint8_t i = 0; i < 10; i++) {
		MOS::Task::delay(750);
		MOS::GlobalRes::leds[1].toggle();
		MOS::Task::print_name();
	}
	MOS::Task::terminate();
}

void Task2(void* argv = nullptr)
{
	for (uint8_t i = 0; i < 10; i++) {
		MOS::Task::delay(1000);
		MOS::GlobalRes::leds[2].toggle();
		MOS::Task::print_name();
	}
	MOS::Task::terminate();
}

void Task3(void* argv = nullptr)
{
	for (uint8_t i = 0; i < 10; i++) {
		MOS::Task::delay(1500);
		MOS::Task::print_name();
	}
	MOS::Task::terminate();
}

void Task4(void* argv = nullptr)
{
	// Create a sub task
	MOS::Task::create(Task3, nullptr, 4, "S1");

	for (uint8_t i = 0; i < 10; i++) {
		MOS::Task::delay(2000);
		MOS::Task::print_name();
	}
	MOS::Task::terminate();
}

int main(void)
{
	using namespace MOS;

	Resource_Config();

	// Create idle task
	Task::create(idle, nullptr, 15, "IDIE");

	// Create user tasks
	Task::create(Task0, nullptr, 0, "T0");
	Task::create(Task1, nullptr, 1, "T1");
	Task::create(Task2, nullptr, 2, "T2");
	Task::create(Task3, nullptr, 3, "T3");
	Task::create(Task4, nullptr, 4, "T4");

	Scheduler::launch();

	while (true) {
		// loop!()
	}
}