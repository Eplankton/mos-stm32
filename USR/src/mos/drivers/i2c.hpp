#ifndef _MOS_DRIVER_I2C_
#define _MOS_DRIVER_I2C_

#include "../config.h"
#include "gpio.hpp"

namespace MOS::Driver
{
	struct I2C_t : public I2C_TypeDef
	{
		using Self_t      = I2C_t;
		using Raw_t       = I2C_TypeDef*;
		using Init_t      = I2C_InitTypeDef;
		using Flag_t      = const uint32_t;
		using State_t     = FunctionalState;
		using Event_t     = const uint32_t;
		using Addr_t      = const uint8_t;
		using Direction_t = const uint8_t;
		using Data_t      = const uint8_t;
		using RawPort_t   = GPIO_t::Raw_t;
		using PinSrc_t    = GPIO_t::PinSrc_t;
		using AF_t        = GPIO_t::AF_t;
		using Speed_t     = GPIO_t::Speed_t;

		I2C_t()                  = delete;
		I2C_t(const Self_t& src) = delete;

		// Functions
		static constexpr inline I2C_t&
		convert(Raw_t I2Cx) { return (Self_t&) (*I2Cx); }

		inline constexpr Raw_t get_raw() { return this; }

		inline constexpr auto&
		init(Init_t&& cfg)
		{
			I2C_Init(this, &cfg);
			return *this;
		}

		inline constexpr auto&
		cmd(State_t new_state)
		{
			I2C_Cmd(this, new_state);
			return *this;
		}

		inline constexpr auto
		get_flag_status(Flag_t i2c_flag) const
		{
			return I2C_GetFlagStatus((Raw_t) this, i2c_flag);
		}

		inline constexpr auto&
		ack_config(State_t new_state)
		{
			I2C_AcknowledgeConfig(this, new_state);
			return *this;
		}

		inline constexpr auto&
		attach(RawPort_t port, PinSrc_t src,
		       AF_t af, Speed_t speed = GPIO_Fast_Speed)
		{
			GPIO_t::convert(port)
			        .init(GPIO_t::get_pin(src),
			              GPIO_Mode_AF,
			              speed,
			              GPIO_OType_OD,
			              GPIO_PuPd_NOPULL)
			        .pin_af_config(src, af);
			return *this;
		}

		inline constexpr auto&
		scl_config(RawPort_t scl_port, PinSrc_t scl_src,
		           AF_t scl_af, Speed_t scl_speed = GPIO_Fast_Speed)
		{
			return attach(scl_port, scl_src, scl_af);
		}

		inline constexpr auto&
		sda_config(RawPort_t sda_port, PinSrc_t sda_src,
		           AF_t sda_af, Speed_t sda_speed = GPIO_Fast_Speed)
		{
			return attach(sda_port, sda_src, sda_af);
		}

		inline constexpr auto&
		generate_start(State_t new_state)
		{
			I2C_GenerateSTART(this, new_state);
			return *this;
		}

		inline constexpr auto&
		generate_stop(State_t new_state)
		{
			I2C_GenerateSTOP(this, new_state);
			return *this;
		}

		inline constexpr auto
		check_event(Event_t i2c_event)
		{
			return I2C_CheckEvent(this, i2c_event);
		}

		inline constexpr auto&
		send_7bit_addr(Addr_t addr, Direction_t i2c_direction)
		{
			I2C_Send7bitAddress(this, addr, i2c_direction);
			return *this;
		}

		inline constexpr auto&
		send_data(Data_t data)
		{
			I2C_SendData(this, data);
			return *this;
		}

		inline constexpr auto
		receive_data() const
		{
			return I2C_ReceiveData((Raw_t) this);
		}
	};
}

#endif