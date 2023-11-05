#ifndef _MOS_DRIVER_SPI_
#define _MOS_DRIVER_SPI_

#include "../config.h"
#include "gpio.hpp"

namespace MOS::Driver
{
	struct SPI_t : public SPI_TypeDef
	{
		using Self_t    = SPI_t;
		using Raw_t     = SPI_TypeDef*;
		using Init_t    = SPI_InitTypeDef;
		using Flag_t    = const uint16_t;
		using State_t   = FunctionalState;
		using RawPort_t = GPIO_t::Raw_t;
		using PinSrc_t  = GPIO_t::PinSrc_t;
		using AF_t      = GPIO_t::AF_t;
		using Speed_t   = GPIO_t::Speed_t;
		using Data_t    = const uint8_t;

		SPI_t()                  = delete;
		SPI_t(const Self_t& src) = delete;

		// Functions
		static constexpr inline SPI_t&
		convert(Raw_t SPIx) { return (Self_t&) (*SPIx); }

		inline constexpr Raw_t get_raw() { return this; }

		inline constexpr auto&
		init(Init_t&& cfg)
		{
			SPI_Init(this, &cfg);
			return *this;
		}

		inline constexpr auto&
		attach(GPIO_t::Raw_t port, PinSrc_t src, AF_t af,
		       Speed_t speed = GPIO_High_Speed)
		{
			GPIO_t::convert(port)
			        .init(GPIO_t::get_pin(src),
			              GPIO_Mode_AF,
			              speed,
			              GPIO_OType_PP,
			              GPIO_PuPd_UP)
			        .pin_af_config(src, af);
			return *this;
		}

		inline constexpr auto&
		sclk_config(GPIO_t::Raw_t sclk_port,
		            PinSrc_t sclk_src, AF_t sclk_af,
		            Speed_t sclk_speed = GPIO_High_Speed)
		{
			return attach(sclk_port, sclk_src, sclk_af, sclk_speed);
		}

		inline constexpr auto&
		mosi_config(GPIO_t::Raw_t mosi_port,
		            PinSrc_t mosi_src, AF_t mosi_af,
		            Speed_t mosi_speed = GPIO_High_Speed)
		{
			return attach(mosi_port, mosi_src, mosi_af, mosi_speed);
		}

		inline constexpr auto&
		cmd(State_t new_state)
		{
			SPI_Cmd(this, new_state);
			return *this;
		}

		inline constexpr auto&
		enable()
		{
			return cmd(ENABLE);
		}

		inline constexpr auto&
		disable()
		{
			return cmd(DISABLE);
		}

		// Do inline this function
		__attribute__((noinline)) constexpr auto&
		send_data(Data_t data)
		{
			SPI_SendData(this, data);
			return *this;
		}

		inline constexpr auto
		get_flag_status(Flag_t flag) const
		{
			return SPI_GetFlagStatus((Raw_t) this, flag);
		}

		inline void write_bus(uint8_t data)
		{
			send_data(data);
			while (!get_flag_status(SPI_I2S_FLAG_TXE))
				;
		}
	};
}

#endif