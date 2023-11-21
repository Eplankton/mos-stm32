#ifndef _DRIVER_DMA_
#define _DRIVER_DMA_

#include "stm32f4xx_dma.h"

namespace HAL::STM32F4xx
{
	struct DMA_Stream_t : public DMA_Stream_TypeDef
	{
		using Self_t  = DMA_Stream_t;
		using Raw_t   = DMA_Stream_TypeDef*;
		using Init_t  = DMA_InitTypeDef;
		using Flag_t  = const uint32_t;
		using State_t = FunctionalState;

		DMA_Stream_t()                  = delete;
		DMA_Stream_t(const Self_t& src) = delete;

		// Functions
		static constexpr inline DMA_Stream_t&
		convert(Raw_t DMAy_Streamx) { return (Self_t&) (*DMAy_Streamx); }

		inline constexpr Raw_t get_raw() { return this; }

		inline constexpr auto&
		init(Init_t&& cfg)
		{
			DMA_Init(this, &cfg);
			return *this;
		}

		inline constexpr auto&
		deinit()
		{
			DMA_DeInit(this);
			return *this;
		}

		inline constexpr auto
		get_cmd_status() const
		{
			return DMA_GetCmdStatus((Raw_t) this);
		}

		inline constexpr auto
		get_flag_status(Flag_t flag) const
		{
			return DMA_GetFlagStatus((Raw_t) this, flag);
		}

		inline constexpr void
		clear_flag(Flag_t flag)
		{
			DMA_ClearFlag(this, flag);
		}

		inline constexpr auto&
		cmd(State_t new_state)
		{
			DMA_Cmd(this, new_state);
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
	};
}

#endif