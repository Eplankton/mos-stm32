#include "main.h"
#include "mos/kernel.hpp"

// Put all global resource here
namespace MOS::GlobalRes
{
	// Serial input and output
	auto& uart = convert(USART3);

	// LED red, green, blue
	LED_t leds[] = {
	        {GPIOB, GPIO_Pin_14},
	        {GPIOB,  GPIO_Pin_0},
	        {GPIOB,  GPIO_Pin_7},
	};
}

namespace MOS::Bsp
{
	// For printf_
	extern "C" void _putchar(char ch)
	{
		using GlobalRes::uart;
		uart.send_data(ch);                                   /* 发送一个字节数据到串口 */
		while (uart.get_flag_status(USART_FLAG_TXE) == RESET) /* 等待发送完毕 */
			;
	}

	static inline void NVIC_Config()
	{
		NVIC_t::group_config(NVIC_PriorityGroup_2);
	}

	static inline void LED_Config()
	{
		using GlobalRes::leds;
		RCC_t::AHB1::clock_cmd(RCC_AHB1Periph_GPIOB, ENABLE);
		for (auto& led: leds) {
			led.init();
		}
	}

	static inline void K1_IRQ_Config()
	{
		RCC_t::AHB1::clock_cmd(RCC_AHB1Periph_GPIOC, ENABLE);
		RCC_t::APB2::clock_cmd(RCC_APB2Periph_SYSCFG, ENABLE);
		SYSCFG_t::exti_line_config(EXTI_PortSourceGPIOC, EXTI_PinSource13);
		EXTI_t::init(EXTI_Line13, EXTI_Mode_Interrupt, EXTI_Trigger_Rising, ENABLE);
		NVIC_t::init(EXTI15_10_IRQn, 1, 1, ENABLE);
	}

	// K1 IRQ Handler
	extern "C" void EXTI15_10_IRQHandler()
	{
		using namespace MOS;
		EXTI_t::handle_line(EXTI_Line13, [] {
			printf("[MOS]: K1 IRQ!\n");
			Task::print_all_tasks();
		});
	}

	static inline void USART_Config()
	{
		using GlobalRes::uart;

		RCC_t::AHB1::clock_cmd(RCC_AHB1Periph_GPIOD, ENABLE);
		RCC_t::APB1::clock_cmd(RCC_APB1Periph_USART3, ENABLE);

		uart.init(9600, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No)
		        .rx_config(GPIOD, GPIO_t::get_pin_src(9), GPIO_AF_USART3)
		        .tx_config(GPIOD, GPIO_t::get_pin_src(8), GPIO_AF_USART3)
		        .enable();
	}

	static inline void config()
	{
		NVIC_Config();
		USART_Config();
		LED_Config();
		K1_IRQ_Config();
	}
}

namespace MOS::App
{
	void Task0(void* argv = nullptr)
	{
		using namespace MOS;

		while (true) {
			Task::delay_ms(500);
			GlobalRes::leds[0].toggle();
			Task::print_name();
			Task::resume(Task::find_by("T4"));
		}
	}

	void Task1(void* argv = nullptr)
	{
		using namespace MOS;

		for (uint8_t i = 0; i < 20; i++) {
			if (i % 2 == 0) {
				Task::delay_ms(750);
				GlobalRes::leds[1].toggle();
				Task::print_name();
			}
			else {
				Task::yield();
			}
		}

		Task::terminate();
	}

	void Task2(void* argv = nullptr)
	{
		using namespace MOS;

		for (uint8_t i = 0; i < 10; i++) {
			Task::delay_ms(1000);
			GlobalRes::leds[2].toggle();
			Task::print_name();
			if (i == 5 && Task::num() < Macro::MAX_TASK_NUM) {
				Task::create(Task2, nullptr, 12, "T2x");
			}
		}

		Task::terminate();
	}

	void Task3(void* argv = nullptr)
	{
		using namespace MOS;

		for (uint8_t i = 0; i < 10; i++) {
			Task::delay_ms(1500);
			Task::print_name();
		}

		Task::terminate();
	}

	void Task4(void* argv = nullptr)
	{
		using namespace MOS;

		Task::create(Task1, nullptr, 0, "S1");

		for (uint8_t i = 0; i < 10; i++) {
			Task::delay_ms(2000);
			Task::print_name();
			if (i == 5) {
				Task::block();
			}
		}

		Task::terminate();
	}
}

void idle(void* argv = nullptr)
{
	using namespace MOS;
	using namespace MOS::App;

	// Create user tasks
	Task::create(Task0, nullptr, 10, "T0");
	Task::create(Task1, nullptr, 1, "T1");
	Task::create(Task2, nullptr, 2, "T2");
	Task::create(Task3, nullptr, 3, "T3");
	Task::create(Task4, nullptr, 4, "T4");

	// Print tasks
	Task::print_all_tasks();

	while (true) {
		// Idle does nothing but loop...
		Task::delay_ms(1000);
		Task::print_name();
	}
}

static inline void Welcome()
{
	using namespace MOS;
	printf("[MOS]: Hello :)  Build Time: %s, %s\n", __TIME__, __DATE__);
}

int main(void)
{
	using namespace MOS;

	// Init resource
	Bsp::config();

	// Show slogan
	Welcome();

	// Create idle task
	Task::create(idle, nullptr, 15, "idle");

	// Start Scheduling
	Scheduler::launch();

	while (true) {
		// loop!()
	}
}