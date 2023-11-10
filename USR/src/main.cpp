#include "main.h"
#include "mos/kernel.hpp"
#include "mos/shell.hpp"

// Put user global data here
namespace MOS::UserGlobal
{
	// Serial input and output
	auto& uart = Driver::convert(USART3);

	// LED red, green, blue
	Driver::LED_t leds[] = {
	        {GPIOB, GPIO_Pin_14},
	        {GPIOB,  GPIO_Pin_0},
	        {GPIOB,  GPIO_Pin_7},
	};

	Driver::ST7735S lcd {
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
	using namespace Driver;

	// For printf_
	extern "C" void _putchar(char ch)
	{
		uart.send_data(ch);
		while (uart.get_flag_status(USART_FLAG_TXE) == RESET)
			;
	}

	static inline void LED_Config()
	{
		RCC_t::AHB1::clock_cmd(RCC_AHB1Periph_GPIOB, ENABLE);
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
		RCC_t::AHB1::clock_cmd(RCC_AHB1Periph_GPIOC, ENABLE);
		RCC_t::APB2::clock_cmd(RCC_APB2Periph_SYSCFG, ENABLE);
		SYSCFG_t::exti_line_config(EXTI_PortSourceGPIOC, EXTI_PinSource13);
		EXTI_t::init(EXTI_Line13, EXTI_Mode_Interrupt, EXTI_Trigger_Rising, ENABLE);
		NVIC_t::init(EXTI15_10_IRQn, 1, 1, ENABLE);
	}

	static inline void USART_Config()
	{
		RCC_t::AHB1::clock_cmd(RCC_AHB1Periph_GPIOD, ENABLE);
		RCC_t::APB1::clock_cmd(RCC_APB1Periph_USART3, ENABLE);
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
		MOS_DISABLE_IRQ();
		os_ticks++;
		MOS_TRIGGER_PENDSV_INTR();
		MOS_ENABLE_IRQ();
	}

	// K1 IRQ Handler
	extern "C" void EXTI15_10_IRQHandler()
	{
		using UserGlobal::leds;
		using Driver::EXTI_t;

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
		using Color = Driver::ST7735S::Color;
		using UserGlobal::lcd;

		// constexpr auto logo = " A_A       _\n"
		//                       "o'' )_____//\n"
		//                       " `_/  MOS  )\n"
		//                       " (_(_/--(_/\n";

		while (true) {
			lcd.show_string(1, 1, "Hello, World!", Color::GREEN);
			Task::delay(250);
			lcd.clear(lcd.bkgd);

			lcd.test_gray16();
			Task::delay(250);
			lcd.clear(lcd.bkgd);
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

void idle(void* argv)
{
	using namespace MOS;

	// Create user tasks
	Task::create(Shell::launch, nullptr, 1, "Shell");
	Task::create(App::LCD, nullptr, 1, "LCD");
	Task::create(App::Task0, nullptr, 1, "T0");

	// Task::create(App::MutexTest, nullptr, 1, "T1");
	// Task::create(App::MutexTest, nullptr, 2, "T2");
	// Task::create(App::MutexTest, nullptr, 3, "T3");

	// Set idle to the lowest priority
	Task::change_priority(Task::current_task(), Macro::PRI_MIN);

	while (true) {
		// nothing but loop...
		asm volatile("");
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