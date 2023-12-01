#ifndef _DRIVER_EXTI_
#define _DRIVER_EXTI_

#include "stm32f4xx_exti.h"

namespace HAL::STM32F4xx
{
	struct EXTI_t : public EXTI_TypeDef
	{
		// Type alias
		using Self_t    = EXTI_t;
		using Raw_t     = EXTI_TypeDef*;
		using Init_t    = EXTI_InitTypeDef;
		using Line_t    = const uint32_t;
		using Mode_t    = const EXTIMode_TypeDef;
		using Trigger_t = const EXTITrigger_TypeDef;
		using State_t   = FunctionalState;

		EXTI_t()                  = delete;
		EXTI_t(const Self_t& src) = delete;

		// Functions
		static inline constexpr EXTI_t&
		convert(Raw_t raw) { return (Self_t&) (*raw); }

		static inline void
		init(Init_t&& cfg)
		{
			EXTI_Init(&cfg);
		}

		static inline void
		init(Line_t line, Mode_t mode, Trigger_t trigger, State_t new_state)
		{
			init(Init_t {line, mode, trigger, new_state});
		}

		static inline auto
		get_status(Line_t line)
		{
			return EXTI_GetITStatus(line);
		}

		static inline void
		clear_pending_bit(Line_t line)
		{
			EXTI_ClearITPendingBit(line);
		}

		static inline void
		generate_swi(Line_t line)
		{
			EXTI_GenerateSWInterrupt(line);
		}

		__attribute__((always_inline)) static inline bool
		handle_line(Line_t line, auto&& fn)
		{
			if (get_status(line)) {
				fn();
				clear_pending_bit(line);
				return true;
			}
			else {
				return false;
			}
		}
	};
}

#endif