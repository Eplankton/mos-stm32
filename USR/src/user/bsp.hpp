#ifndef _MOS_USER_BSP_
#define _MOS_USER_BSP_

// STM32F4xx HAL
#include "src/drivers/stm32f4xx/hal.hpp"
#include "src/mos/kernel/task.hpp"
#include "src/user/global.hpp"

namespace MOS::Bsp
{
	using namespace HAL::STM32F4xx;

	// For printf_
	extern "C" void _putchar(char ch)
	{
		using UserGlobal::uart3;
		uart3.send_data(ch);
		uart3.wait_flag(USART_FLAG_TXE);
	}

	static inline void LED_Config()
	{
		using UserGlobal::leds;
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
		SYSCFG_t::exti_line_config(EXTI_PortSourceGPIOC, EXTI_PinSource13); // K1 -> PC13
		EXTI_t::init(EXTI_Line13, EXTI_Mode_Interrupt, EXTI_Trigger_Rising, ENABLE);
		NVIC_t::init(EXTI15_10_IRQn, 1, 1, ENABLE);
	}

	static inline void USART_Config()
	{
		using UserGlobal::uart3;

		RCC_t::AHB1::enable(RCC_AHB1Periph_GPIOD);
		RCC_t::APB1::enable(RCC_APB1Periph_USART3);
		NVIC_t::init(USART3_IRQn, 1, 1, ENABLE);

		uart3.init(9600, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No)
		        .rx_config(GPIOD, GPIO_t::get_pin_src(9), GPIO_AF_USART3) // RX -> PD9
		        .tx_config(GPIOD, GPIO_t::get_pin_src(8), GPIO_AF_USART3) // TX -> PD8
		        .it_enable(USART_IT_RXNE)
		        .enable();
	}

	static inline void LCD_Config()
	{
		using UserGlobal::lcd;

		RCC_t::APB2::enable(RCC_APB2Periph_SPI1);
		RCC_t::AHB1::enable(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD);

		lcd.spi.sclk_config(GPIOA, GPIO_t::get_pin_src(5), GPIO_AF_SPI1)   // SCLK -> PA5
		        .mosi_config(GPIOA, GPIO_t::get_pin_src(7), GPIO_AF_SPI1); // MOSI -> PA7
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
	// K1 IRQ Handler
	extern "C" void EXTI15_10_IRQHandler()
	{
		using HAL::STM32F4xx::EXTI_t;
		using UserGlobal::leds;

		static auto K1_IRQ = [](void* argv) {
			for (uint8_t i = 0; i < 10; i++) {
				leds[2].toggle();
				Task::print_name();
				Task::delay(100);
			}
		};

		static uint32_t k1_cnt = 0;

		EXTI_t::handle_line(EXTI_Line13, [] {
			MOS_MSG("K1 Cnt = %d\n", ++k1_cnt);
			Task::create_fromISR(
			        K1_IRQ,
			        nullptr,
			        Task::current()->get_priority(),
			        "K1");
		});
	}

	// UART3 IRQ Handler
	extern "C" void USART3_IRQHandler()
	{
		using UserGlobal::uart3;
		using UserGlobal::rx_buf;

		if (uart3.get_it_status(USART_IT_RXNE) != RESET) {
			char8_t data = uart3.receive_data();
			if (!rx_buf.full()) {
				rx_buf.add(data);
			}
			else {
				rx_buf.clear();
				MOS_MSG("Oops! Command too long!\n\n");
			}
		}
	}
}

#endif