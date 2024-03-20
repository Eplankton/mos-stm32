#ifndef _DRIVER_SPI_
#define _DRIVER_SPI_

#include "gpio.hpp"
#include "stm32f4xx_spi.h"

namespace HAL::STM32F4xx
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
		static inline constexpr SPI_t&
		convert(Raw_t SPIx) { return (Self_t&) (*SPIx); }

		inline Raw_t get_raw() { return this; }

		inline auto&
		init(Init_t&& cfg)
		{
			SPI_Init(this, &cfg);
			return *this;
		}

		inline auto&
		attach(
		    GPIO_t::Raw_t port,
		    PinSrc_t src,
		    AF_t af,
		    Speed_t speed = GPIO_High_Speed
		)
		{
			GPIO_t::convert(port)
			    .init(
			        GPIO_t::get_pin(src),
			        GPIO_Mode_AF,
			        speed,
			        GPIO_OType_PP,
			        GPIO_PuPd_UP
			    )
			    .pin_af_config(src, af);
			return *this;
		}

		inline auto&
		sclk_config(
		    GPIO_t::Raw_t sclk_port,
		    PinSrc_t sclk_src,
		    AF_t sclk_af,
		    Speed_t sclk_speed = GPIO_High_Speed
		)
		{
			return attach(
			    sclk_port,
			    sclk_src,
			    sclk_af,
			    sclk_speed
			);
		}

		inline auto&
		mosi_config(
		    GPIO_t::Raw_t mosi_port,
		    PinSrc_t mosi_src,
		    AF_t mosi_af,
		    Speed_t mosi_speed = GPIO_High_Speed
		)
		{
			return attach(
			    mosi_port,
			    mosi_src,
			    mosi_af,
			    mosi_speed
			);
		}

		inline auto&
		miso_config(
		    GPIO_t::Raw_t miso_port,
		    PinSrc_t miso_src,
		    AF_t miso_af,
		    Speed_t miso_speed = GPIO_High_Speed
		)
		{
			return attach(
			    miso_port,
			    miso_src,
			    miso_af,
			    miso_speed
			);
		}

		inline auto&
		cmd(State_t new_state)
		{
			SPI_Cmd(this, new_state);
			return *this;
		}

		inline auto&
		enable()
		{
			return cmd(ENABLE);
		}

		inline auto&
		disable()
		{
			return cmd(DISABLE);
		}

		__attribute__((always_inline)) inline auto
		get_flag_status(Flag_t flag) const
		{
			return SPI_GetFlagStatus((Raw_t) this, flag);
		}

		__attribute__((noinline)) auto&
		send_data(Data_t data)
		{
			SPI_SendData(this, data);
			return *this;
		}

		__attribute__((noinline)) auto
		recv_data() const
		{
			return SPI_ReceiveData((Raw_t) this);
		}

		__attribute__((noinline)) void
		write_bus(uint8_t data)
		{
			send_data(data);
			while (!get_flag_status(SPI_I2S_FLAG_TXE))
				;
		}
	};
}

#endif