#ifndef _NUTS_NVIC_
#define _NUTS_NVIC_

#include "stm32f4xx.h"

namespace nuts
{
	struct NVIC_t : public NVIC_Type
	{
		// Type alias
		using Self_t    = NVIC_t;
		using Raw_t     = NVIC_Type*;
		using Init_t    = NVIC_InitTypeDef;
		using Channel_t = const uint8_t;
		using Prior_t   = const uint8_t;
		using State_t   = FunctionalState;
		using Group_t   = const uint32_t;

		NVIC_t()                  = delete;
		NVIC_t(const Self_t& src) = delete;

		// Functions
		static constexpr inline NVIC_t&
		convert(Raw_t raw) { return (Self_t&) (*raw); }

		static inline void
		init(Init_t&& cfg)
		{
			NVIC_Init(&cfg);
		}

		static inline void
		init(Channel_t channel, Prior_t preempt_pr, Prior_t sub_pr, State_t new_state)
		{
			init(Init_t {channel, preempt_pr, sub_pr, new_state});
		}

		static inline void
		enable(Channel_t channel)
		{
			NVIC_EnableIRQ((IRQn) channel);
		}

		static inline void
		disable(Channel_t channel)
		{
			NVIC_DisableIRQ((IRQn) channel);
		}

		static inline void
		group_config(Group_t pr_group)
		{
			NVIC_PriorityGroupConfig(pr_group);
		}
	};
}

#endif