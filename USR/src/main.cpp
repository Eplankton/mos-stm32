#include "main.h"
#include "mos/kernel.hpp"
#include "mos/shell.hpp"

// Put user global data here
namespace MOS::UserGlobal
{
	using namespace DataType;

	// Serial input and output
	auto& uart = Driver::convert(USART3);

	// LED red, green, blue
	Driver::LED_t leds[] = {
	        {GPIOB, GPIO_Pin_14},
	        {GPIOB,  GPIO_Pin_0},
	        {GPIOB,  GPIO_Pin_7},
	};
}

namespace MOS::Bsp
{
	using namespace UserGlobal;
	using namespace Driver;

	// For printf_
	extern "C" void _putchar(char ch)
	{
		uart.send_data(ch);                                   /* 发送一个字节数据到串口 */
		while (uart.get_flag_status(USART_FLAG_TXE) == RESET) /* 等待发送完毕 */
			;
	}

	static inline void LED_Config()
	{
		RCC_t::AHB1::clock_cmd(RCC_AHB1Periph_GPIOB, ENABLE);
		for (auto& led: leds) {
			led.init();
		}
	}

	static inline void NVIC_Config()
	{
		NVIC_t::group_config(NVIC_PriorityGroup_2);
		NVIC_t::init(EXTI15_10_IRQn, 1, 1, ENABLE);
		NVIC_t::init(USART3_IRQn, 1, 1, ENABLE);
	}

	static inline void K1_IRQ_Config()
	{
		RCC_t::AHB1::clock_cmd(RCC_AHB1Periph_GPIOC, ENABLE);
		RCC_t::APB2::clock_cmd(RCC_APB2Periph_SYSCFG, ENABLE);
		SYSCFG_t::exti_line_config(EXTI_PortSourceGPIOC, EXTI_PinSource13);
		EXTI_t::init(EXTI_Line13, EXTI_Mode_Interrupt, EXTI_Trigger_Rising, ENABLE);
	}

	static inline void USART_Config()
	{
		RCC_t::AHB1::clock_cmd(RCC_AHB1Periph_GPIOD, ENABLE);
		RCC_t::APB1::clock_cmd(RCC_APB1Periph_USART3, ENABLE);

		uart.init(9600, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No)
		        .rx_config(GPIOD, GPIO_t::get_pin_src(9), GPIO_AF_USART3)
		        .tx_config(GPIOD, GPIO_t::get_pin_src(8), GPIO_AF_USART3)
		        .it_enable(USART_IT_RXNE)
		        .enable();
	}

	static inline void config()
	{
		LED_Config();
		NVIC_Config();
		USART_Config();
		K1_IRQ_Config();
	}
}

namespace MOS::IRQ
{
	// K1 IRQ Handler
	extern "C" void EXTI15_10_IRQHandler()
	{
		static auto K1_IRQ = [](void* argv) {
			for (uint32_t i = 0; i < 5; i++) {
				Task::delay_ms(1000);
				UserGlobal::leds[2].toggle();
				Task::print_name();
			}
		};

		Driver::EXTI_t::handle_line(EXTI_Line13, [] {
			MOS_MSG("[MOS]: K1 IRQ!\n");
			Task::create(K1_IRQ, nullptr, 1, "K1");
			Task::print_all_tasks();
		});
	}

	// UART3 IRQ Handler
	extern "C" void USART3_IRQHandler(void)
	{
		using KernelGlobal::rx_buf;
		using UserGlobal::uart;

		if (uart.get_it_status(USART_IT_RXNE) != RESET) {
			char data = uart.receive_data();
			if (data != '\n' && !rx_buf.full()) {
				rx_buf.add(data);
			}
		}
	}
}

namespace MOS::App
{
	using UserGlobal::leds;

	void Task1(void* argv)
	{
		for (uint32_t i = 0; i < 50; i++) {
			Task::delay_ms(500);
			leds[1].toggle();
		}
	}

	void Task0(void* argv)
	{
		Task::create(Task1, nullptr, 1, "T1");
		while (true) {
			Task::delay_ms(500);
			leds[0].toggle();
		}
	}
}

void idle(void* argv)
{
	using namespace MOS;

	// Create user tasks
	Task::create(Shell::launch, nullptr, 1, "Shell");
	Task::create(App::Task0, nullptr, 1, "T0");

	// Print tasks
	Task::print_all_tasks();

	// Set idle to the lowest priority
	Task::change_priority(Task::current_task(), Macro::PRI_MIN);

	while (true) {
		// Idle does nothing but loop...
		Task::delay_ms(1000);
		// Task::print_name();
	}
}

int main(void)
{
	using namespace MOS;

	// Init resource
	Bsp::config();

	// Create idle task
	Task::create(idle, nullptr, Macro::PRI_MAX, "idle");

	// Start scheduling, never return
	Scheduler::launch();

	while (true) {
		// Never comes here
	}
}