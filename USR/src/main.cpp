#include "main.h"
#include "mos/kernel.hpp"

// Put all global resource here
namespace MOS::GlobalRes
{
	using namespace Driver;

	// Serial input and output
	auto& uart = convert(USART3);

	// LED red, green, blue
	LED_t leds[] = {
	        {GPIOB, GPIO_Pin_14},
	        {GPIOB,  GPIO_Pin_0},
	        {GPIOB,  GPIO_Pin_7},
	};

	Sync::Semaphore_t sema {1};
}

namespace MOS::Bsp
{
	using namespace Driver;

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
	using namespace GlobalRes;

	void Task1(void* argv)
	{
		for (uint32_t i = 0; i < 20; i++) {
			Task::delay_ms(500);
			leds[1].toggle();
			Task::print_name();
			if (i == 10) {
				Task::change_priority(Task::current_task(), 3);
			}
		}
	}

	void Task0(void* argv)
	{
		for (uint32_t i = 0; i < 20; i++) {
			Task::delay_ms(500);
			leds[0].toggle();
			Task::print_name();
			if (i == 10) {
				Task::create(Task1, nullptr, 1, "T1");
			}
		}
	}

	void IRQ1(void* argv)
	{
		for (uint32_t i = 0; i < 5; i++) {
			Task::delay_ms(1000);
			leds[2].toggle();
			Task::print_name();
		}
	}

	// K1 IRQ Handler
	extern "C" void EXTI15_10_IRQHandler()
	{
		Driver::EXTI_t::handle_line(EXTI_Line13, [] {
			MOS_MSG("[MOS]: K1 IRQ!\n");
			Task::create(IRQ1, nullptr, 0, "IRQ1");
			Task::print_all_tasks();
		});
	}

	void RoundRobinTest(void* argv)
	{
		for (uint32_t i = 0; i < 10; i++) {
			Task::delay_ms(500);
			Task::print_name();
		}
	}

	int x = 0;
	Sync::Semaphore_t s {1};

	void threadA(void*)
	{
		while (true) {
			s.down();
			x++;
			printf("Thread A: x = %d\n", x);
			Task::delay_ms(1000);
			s.up();
		}
	}

	void threadB(void*)
	{
		while (true) {
			s.down();
			x--;
			printf("Thread B: x = %d\n", x);
			Task::delay_ms(1000);
			s.up();
		}
	}
}

void idle(void* argv)
{
	using namespace MOS;
	using namespace App;

	// Create user tasks
	// Task::create(Task0, nullptr, 2, "T0");

	Task::create(RoundRobinTest, nullptr, 1, "p1");
	Task::create(RoundRobinTest, nullptr, 1, "p2");
	Task::create(RoundRobinTest, nullptr, 1, "p3");

	// Task::create(threadA, nullptr, 1, "A");
	// Task::create(threadB, nullptr, 1, "B");

	// Print tasks
	Task::print_all_tasks();

	// Set idle as the lowest priority
	Task::change_priority(Task::current_task(), Macro::PRI_MIN);

	while (true) {
		// Idle does nothing but loop...
		Task::delay_ms(5000);
		Task::print_name();
	}
}

static inline void Welcome()
{
	MOS_MSG(" A_A       _\n"
	        "o'' )_____//  Build Time = %s, %s\n"
	        " `_/  MOS  )  Policy = %s\n"
	        " (_(_/--(_/\n",
	        __TIME__, __DATE__, MOS::Scheduler::policy_name());
}

int main(void)
{
	using namespace MOS;

	// Init resource
	Bsp::config();

	// Show slogan
	Welcome();

	// Create idle task
	Task::create(idle, nullptr, Macro::PRI_MAX, "idle");

	// Start scheduling, never return
	Scheduler::launch();

	while (true) {
		// Never comes here
	}
}