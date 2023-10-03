#ifndef _NUTS_MOD_
#define _NUTS_MOD_

#include "type.hpp"
#include "concepts.hpp"

#include "gpio.hpp"
#include "delay.hpp"
#include "led.hpp"
#include "key.hpp"
#include "rcc.hpp"
#include "syscfg.hpp"
#include "exti.hpp"
#include "nvic.hpp"
#include "systick.hpp"
#include "usart.hpp"
// #include "io.hpp"
#include "dma.hpp"
#include "i2c.hpp"
#include "tim.hpp"

namespace nuts
{
	template <typename Raw>
	inline constexpr auto&
	convert(Raw)
	{
		// Empty
	}

	template <>
	inline constexpr auto&
	convert(GPIO_TypeDef* GPIOx)
	{
		return GPIO_t::convert(GPIOx);
	}

	template <>
	inline constexpr auto&
	convert(EXTI_TypeDef* base)
	{
		return EXTI_t::convert(base);
	}

	template <>
	inline constexpr auto&
	convert(NVIC_Type* base)
	{
		return NVIC_t::convert(base);
	}

	template <>
	inline constexpr auto&
	convert(SYSCFG_TypeDef* base)
	{
		return SYSCFG_t::convert(base);
	}

	template <>
	inline constexpr auto&
	convert(RCC_TypeDef* base)
	{
		return RCC_t::convert(base);
	}

	template <>
	inline constexpr auto&
	convert(USART_TypeDef* USARTx)
	{
		return USART_t::convert(USARTx);
	}

	template <>
	inline constexpr auto&
	convert(DMA_Stream_TypeDef* DMAy_Stream_x)
	{
		return DMA_Stream_t::convert(DMAy_Stream_x);
	}

	template <>
	inline constexpr auto&
	convert(I2C_TypeDef* I2Cx)
	{
		return I2C_t::convert(I2Cx);
	}

	template <>
	inline constexpr auto&
	convert(SysTick_Type* base)
	{
		return SysTick_t::convert(base);
	}

	template <>
	inline constexpr auto&
	convert(TIM_TypeDef* TIMx)
	{
		return TIM_t::convert(TIMx);
	}
}

#endif