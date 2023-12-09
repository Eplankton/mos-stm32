#ifndef _MOS_USER_BSP_
#define _MOS_USER_BSP_

// STM32F4xx HAL
#include "src/drivers/stm32f4xx/hal.hpp"
#include "src/user/global.hpp"

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
		NVIC_t::init(USART3_IRQn, 0, 1, ENABLE);

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

#endif