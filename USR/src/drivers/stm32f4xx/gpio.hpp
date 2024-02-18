#ifndef _DRIVER_GPIO_
#define _DRIVER_GPIO_

#include "stm32f4xx_gpio.h"

namespace HAL::STM32F4xx
{
	struct GPIO_t : public GPIO_TypeDef
	{
		// Type alias
		using Self_t   = GPIO_t;
		using Raw_t    = GPIO_TypeDef*;
		using Init_t   = GPIO_InitTypeDef;
		using Mode_t   = GPIOMode_TypeDef;
		using Speed_t  = GPIOSpeed_TypeDef;
		using Otype_t  = GPIOOType_TypeDef;
		using PuPd_t   = GPIOPuPd_TypeDef;
		using Pin_t    = const uint16_t;
		using PinSrc_t = const uint16_t;
		using AF_t     = const uint8_t;

		GPIO_t()                  = delete;
		GPIO_t(const Self_t& src) = delete;

		// Functions
		static inline constexpr GPIO_t&
		convert(Raw_t GPIOx) { return (Self_t&) (*GPIOx); }

		inline Raw_t get_raw() { return this; }

		inline auto&
		init(Init_t&& cfg)
		{
			GPIO_Init(this, &cfg);
			return *this;
		}

		inline auto&
		init(
		    Pin_t pin,
		    Mode_t mode,
		    Speed_t speed,
		    Otype_t otype,
		    PuPd_t pupd
		)
		{
			return init(
			    Init_t {pin, mode, speed, otype, pupd}
			);
		}

		inline auto&
		as_output(
		    Pin_t pin,
		    Speed_t speed = GPIO_Low_Speed,
		    Otype_t otype = GPIO_OType_PP,
		    PuPd_t pupd   = GPIO_PuPd_UP
		)
		{
			return init(
			    pin, GPIO_Mode_OUT, speed, otype, pupd
			);
		}

		inline auto&
		as_input(Pin_t pin, PuPd_t pupd = GPIO_PuPd_NOPULL)
		{
			return init(Init_t {
			    .GPIO_Pin  = pin,
			    .GPIO_Mode = GPIO_Mode_IN,
			    .GPIO_PuPd = pupd,
			});
		}

		inline void
		set_bits(Pin_t pin)
		{
			GPIO_SetBits(this, pin);
		}

		inline void
		reset_bits(Pin_t pin)
		{
			GPIO_ResetBits(this, pin);
		}

		inline void
		toggle_bits(Pin_t pin)
		{
			GPIO_ToggleBits(this, pin);
		}

		inline void
		lock_pin(Pin_t pin)
		{
			GPIO_PinLockConfig(this, pin);
		}

		inline void
		write_bit(Pin_t pin, BitAction bit_val)
		{
			GPIO_WriteBit(this, pin, bit_val);
		}

		inline void
		write(uint16_t port_val)
		{
			GPIO_Write(this, port_val);
		}

		inline uint8_t
		read_input_bit(Pin_t pin) const
		{
			return GPIO_ReadInputDataBit((Raw_t) this, pin);
		}

		inline uint16_t
		read_input_data() const
		{
			return GPIO_ReadInputData((Raw_t) this);
		}

		inline uint8_t
		read_output_bit(Pin_t pin) const
		{
			return GPIO_ReadOutputDataBit((Raw_t) this, pin);
		}

		inline uint16_t
		read_output_data() const
		{
			return GPIO_ReadOutputData((Raw_t) this);
		}

		inline void
		pin_af_config(PinSrc_t pin_src, AF_t af)
		{
			GPIO_PinAFConfig(this, pin_src, af);
		}

		static constexpr inline Pin_t
		get_pin(auto&&... pin)
		{
			return (... | (1U << pin));
		}

		static constexpr inline PinSrc_t
		get_pin_src(auto&&... pin_src)
		{
			return (... | (uint8_t) pin_src);
		}
	};
}

#endif