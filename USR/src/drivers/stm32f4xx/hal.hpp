#ifndef _DRIVER_HAL_STM32F4xx_
#define _DRIVER_HAL_STM32F4xx_

// STM32F4xx Periphs
#include "gpio.hpp"
#include "rcc.hpp"
#include "syscfg.hpp"
#include "exti.hpp"
#include "nvic.hpp"
#include "systick.hpp"
#include "usart.hpp"
#include "dma.hpp"
#include "tim.hpp"
#include "i2c.hpp"
#include "spi.hpp"
#include "pwr.hpp"
#include "rtc.hpp"

namespace HAL::STM32F4xx
{
	template <typename Raw>
	__attribute__((always_inline)) inline constexpr auto&
	convert(Raw raw)
	{
		// need specialization
	}

	template <>
	__attribute__((always_inline)) inline constexpr auto&
	convert(GPIO_TypeDef* GPIOx)
	{
		return GPIO_t::convert(GPIOx);
	}

	template <>
	__attribute__((always_inline)) inline constexpr auto&
	convert(EXTI_TypeDef* base)
	{
		return EXTI_t::convert(base);
	}

	template <>
	__attribute__((always_inline)) inline constexpr auto&
	convert(NVIC_Type* base)
	{
		return NVIC_t::convert(base);
	}

	template <>
	__attribute__((always_inline)) inline constexpr auto&
	convert(SYSCFG_TypeDef* base)
	{
		return SYSCFG_t::convert(base);
	}

	template <>
	__attribute__((always_inline)) inline constexpr auto&
	convert(RCC_TypeDef* base)
	{
		return RCC_t::convert(base);
	}

	template <>
	__attribute__((always_inline)) inline constexpr auto&
	convert(USART_TypeDef* USARTx)
	{
		return USART_t::convert(USARTx);
	}

	template <>
	__attribute__((always_inline)) inline constexpr auto&
	convert(DMA_Stream_TypeDef* DMAy_Stream_x)
	{
		return DMA_Stream_t::convert(DMAy_Stream_x);
	}

	template <>
	__attribute__((always_inline)) inline constexpr auto&
	convert(I2C_TypeDef* I2Cx)
	{
		return I2C_t::convert(I2Cx);
	}

	template <>
	__attribute__((always_inline)) inline constexpr auto&
	convert(SysTick_Type* base)
	{
		return SysTick_t::convert(base);
	}

	template <>
	__attribute__((always_inline)) inline constexpr auto&
	convert(TIM_TypeDef* TIMx)
	{
		return TIM_t::convert(TIMx);
	}

	template <>
	__attribute__((always_inline)) inline constexpr auto&
	convert(SPI_TypeDef* SPIx)
	{
		return SPI_t::convert(SPIx);
	}

	template <>
	__attribute__((always_inline)) inline constexpr auto&
	convert(RTC_TypeDef* base)
	{
		return RTC_t::convert(base);
	}

	template <>
	__attribute__((always_inline)) inline constexpr auto&
	convert(PWR_TypeDef* base)
	{
		return PWR_t::convert(base);
	}
}

#endif