#include "main.h"

// MOS Kernel & Shell
#include "mos/kernel/kernel.hpp"
#include "mos/shell.hpp"

// STM32F4xx HAL
#include "drivers/stm32f4xx/hal.hpp"

// Devices
#include "drivers/device/st7735s.hpp"
#include "drivers/device/led.hpp"
#include "drivers/device/key.hpp"

// Put user global data here
namespace MOS::UserGlobal
{
	using namespace HAL::STM32F4xx;
	using namespace Driver;

	// Serial input and output
	auto& uart = convert(USART3);

	// LEDs
	LED_t leds[] = {
	        {GPIOB, GPIO_Pin_14}, // red
	        {GPIOB,  GPIO_Pin_0}, // green
	        {GPIOB,  GPIO_Pin_7}, // blue
	};

	ST7735S lcd {
	        SPI1,
	        {GPIOA,  GPIO_Pin_5}, // SCLK(SCL)  -> PA5
	        {GPIOA,  GPIO_Pin_7}, // MOSI(SDA)  -> PA7
	        {GPIOD, GPIO_Pin_14}, // CS(SS)     -> PD14
	        {GPIOB,  GPIO_Pin_8}, // RESET(RES) -> PB8
	        {GPIOB,  GPIO_Pin_9}, // DC(RS)     -> PB9
	};

	Sync::MutexImpl_t mutex;
}

namespace MOS::Bsp
{
	using namespace UserGlobal;
	using namespace HAL::STM32F4xx;

	// For printf_
	extern "C" void _putchar(char ch)
	{
		uart.send_data(ch);
		while (uart.get_flag_status(USART_FLAG_TXE) == RESET)
			;
	}

	static inline void LED_Config()
	{
		RCC_t::AHB1::enable(RCC_AHB1Periph_GPIOB);
		for (auto& led: leds) {
			led.init();
		}
	}

	static inline void NVIC_GroupConfig()
	{
		NVIC_t::group_config(NVIC_PriorityGroup_2);
	}

	static inline void SysTick_Config()
	{
		SysTick_t::config(Macro::SYSTICK);
	}

	static inline void K1_IRQ_Config()
	{
		RCC_t::AHB1::enable(RCC_AHB1Periph_GPIOC);
		RCC_t::APB2::enable(RCC_APB2Periph_SYSCFG);
		SYSCFG_t::exti_line_config(EXTI_PortSourceGPIOC, EXTI_PinSource13);
		EXTI_t::init(EXTI_Line13, EXTI_Mode_Interrupt, EXTI_Trigger_Rising, ENABLE);
		NVIC_t::init(EXTI15_10_IRQn, 1, 1, ENABLE);
	}

	static inline void USART_Config()
	{
		RCC_t::AHB1::enable(RCC_AHB1Periph_GPIOD);
		RCC_t::APB1::enable(RCC_APB1Periph_USART3);
		NVIC_t::init(USART3_IRQn, 1, 1, ENABLE);

		uart.init(9600, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No)
		        .rx_config(GPIOD, GPIO_t::get_pin_src(9), GPIO_AF_USART3)
		        .tx_config(GPIOD, GPIO_t::get_pin_src(8), GPIO_AF_USART3)
		        .it_enable(USART_IT_RXNE)
		        .enable();
	}

	static inline void LCD_Config()
	{
		RCC_t::APB2::enable(RCC_APB2Periph_SPI1);
		RCC_t::AHB1::enable(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD);

		lcd.spi.sclk_config(GPIOA, GPIO_t::get_pin_src(5), GPIO_AF_SPI1)
		        .mosi_config(GPIOA, GPIO_t::get_pin_src(7), GPIO_AF_SPI1);
		lcd.init();
	}

	static inline void config()
	{
		NVIC_GroupConfig();
		USART_Config();
		LED_Config();
		K1_IRQ_Config();
		LCD_Config();
		SysTick_Config();
	}
}

namespace MOS::ISR
{
	extern "C" void PendSV_Handler(void)
	{
		asm("B     ContextSwitch");
	}

	extern "C" void SysTick_Handler(void)
	{
		using KernelGlobal::os_ticks;

		// Trigger PendSV
		Util::DisIntrGuard guard;
		os_ticks++;
		MOS_TRIGGER_PENDSV_INTR();
	}

	// K1 IRQ Handler
	extern "C" void EXTI15_10_IRQHandler()
	{
		using UserGlobal::leds;
		using HAL::STM32F4xx::EXTI_t;

		static auto K1_IRQ = [](void* argv) {
			for (uint32_t i = 0; i < 10; i++) {
				leds[2].toggle();
				Task::print_name();
				Task::delay(100);
			}
		};

		EXTI_t::handle_line(EXTI_Line13, [] {
			MOS_MSG("[MOS]: K1 IRQ!\n");
			Task::create(K1_IRQ, nullptr, 1, "K1");
			// Task::print_all_tasks();
		});
	}

	// UART3 IRQ Handler
	extern "C" void USART3_IRQHandler(void)
	{
		using KernelGlobal::rx_buf;
		using UserGlobal::uart;

		if (uart.get_it_status(USART_IT_RXNE) != RESET) {
			char data = uart.receive_data();
			if (!rx_buf.full()) {
				rx_buf.add(data);
			}
			else {
				rx_buf.clear();
				MOS_MSG("[MOS]: Command too long\n\n");
			}
		}
	}
}

namespace MOS::App
{
	void LCD(void* argv)
	{
		using namespace Driver;
		using UserGlobal::lcd;
		using enum ST7735S::Color;

		Terminal terminal {lcd};

		constexpr auto logo = " A_A       _\n"
		                      "o'' )_____//\n"
		                      " `_/  MOS  )\n"
		                      " (_(_/--(_/ \n";

		while (true) {
			terminal.println("Hello, World!", GREEN);
			Task::delay(250);

			terminal.print(logo, YELLOW);
			Task::delay(250);
		}
	}

	void Task1(void* argv)
	{
		using UserGlobal::leds;

		for (uint32_t i = 0; i < 20; i++) {
			leds[1].toggle();
			Task::delay(100);
		}
	}

	void Task0(void* argv)
	{
		using UserGlobal::leds;

		Task::create(App::Task1, nullptr, 1, "T1");
		while (true) {
			leds[0].toggle();
			Task::delay(200);
		}
	}

	void MutexTest(void* argv)
	{
		using UserGlobal::mutex;

		auto cur = Task::current_task();
		while (true) {
			mutex.lock();
			for (uint8_t i = 0; i < 5; ++i) {
				MOS_MSG("%s is working...\n", cur->get_name());
				Task::delay(100);
			}
			mutex.unlock();
			Task::delay(100);
		}
	}
}

int main(void)
{
	using namespace MOS;

	// Init resource
	Bsp::config();

	// Create user tasks
	Task::create(Shell::launch, nullptr, 1, "Shell");
	Task::create(App::LCD, nullptr, 1, "LCD");
	Task::create(App::Task0, nullptr, 1, "T0");

	// Task::create(App::MutexTest, nullptr, 1, "T1");
	// Task::create(App::MutexTest, nullptr, 2, "T2");
	// Task::create(App::MutexTest, nullptr, 3, "T3");

	// Start scheduling, never return
	Scheduler::launch();

	while (true) {
		// Never comes here
	}
}