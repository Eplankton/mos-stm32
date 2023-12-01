#ifndef _DRIVER_TIM_
#define _DRIVER_TIM_

#include "stm32f4xx_tim.h"

namespace HAL::STM32F4xx
{
	struct TIM_t : TIM_TypeDef
	{
		// Type alias
		using Self_t      = TIM_t;
		using Raw_t       = TIM_TypeDef*;
		using BaseInit_t  = TIM_TimeBaseInitTypeDef;
		using Period_t    = const uint32_t;
		using Prescaler_t = const uint16_t;
		using Flag_t      = const uint16_t;
		using IT_t        = const uint16_t;
		using State_t     = FunctionalState;

		TIM_t()                  = delete;
		TIM_t(const Self_t& src) = delete;

		// Functions
		static inline constexpr TIM_t&
		convert(Raw_t TIMx) { return (Self_t&) (*TIMx); }

		inline Raw_t get_raw() { return this; }

		inline auto&
		base_init(BaseInit_t&& cfg)
		{
			TIM_TimeBaseInit(this, &cfg);
			return *this;
		}

		inline auto&
		base_init(Period_t period, Prescaler_t prescaler)
		{
			BaseInit_t base_cfg;
			base_cfg.TIM_Period    = period;
			base_cfg.TIM_Prescaler = prescaler;
			return base_init((BaseInit_t&&) base_cfg);
		}

		inline auto&
		clear_flag(Flag_t flag)
		{
			TIM_ClearFlag(this, flag);
			return *this;
		}

		inline auto&
		it_config(IT_t tim_it, State_t new_state)
		{
			TIM_ITConfig(this, tim_it, new_state);
			return *this;
		}

		inline auto
		get_it_status(IT_t tim_it) const
		{
			return TIM_GetITStatus((Raw_t) this, tim_it);
		}

		inline auto&
		cmd(State_t new_state)
		{
			TIM_Cmd(this, new_state);
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

		inline auto&
		clear_pending_bit(IT_t tim_it)
		{
			TIM_ClearITPendingBit(this, tim_it);
			return *this;
		}

		inline bool
		handle_it(IT_t tim_it, auto&& fn)
		{
			if (get_it_status(tim_it)) {
				fn();
				clear_pending_bit(tim_it);
				return true;
			}
			else {
				return false;
			}
		}
	};
}

#endif
