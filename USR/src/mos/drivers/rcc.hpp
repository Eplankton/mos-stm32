#ifndef _MOS_DRIVER_RCC_
#define _MOS_DRIVER_RCC_

#include "../config.h"

namespace MOS::Driver
{
	struct RCC_t : public RCC_TypeDef
	{
		// Type alias
		using Self_t   = RCC_t;
		using Raw_t    = RCC_TypeDef*;
		using Periph_t = const uint32_t;
		using State_t  = FunctionalState;

		// Functions
		RCC_t()                  = delete;
		RCC_t(const Self_t& src) = delete;

		static constexpr inline RCC_t&
		convert(Raw_t raw) { return (Self_t&) (*raw); }

		inline constexpr Raw_t get_raw() { return this; }

		struct AHB1
		{
			static inline constexpr void
			clock_cmd(Periph_t ahb1_periphs, State_t new_state)
			{
				RCC_AHB1PeriphClockCmd(ahb1_periphs, new_state);
			}

			static inline constexpr void
			enable(auto&&... ahb1_periphs)
			{
				clock_cmd((... | ahb1_periphs), ENABLE);
			}
		};

		struct AHB2
		{
			static inline constexpr void
			clock_cmd(Periph_t ahb2_periphs, State_t new_state)
			{
				RCC_AHB2PeriphClockCmd(ahb2_periphs, new_state);
			}

			static inline constexpr void
			enable(auto&&... ahb2_periphs)
			{
				clock_cmd((... | ahb2_periphs), ENABLE);
			}
		};

		struct AHB3
		{
			static inline constexpr void
			clock_cmd(Periph_t ahb3_periphs, State_t new_state)
			{
				RCC_AHB3PeriphClockCmd(ahb3_periphs, new_state);
			}

			static inline constexpr void
			enable(auto&&... ahb3_periphs)
			{
				clock_cmd((... | ahb3_periphs), ENABLE);
			}
		};

		struct APB1
		{
			static inline constexpr void
			clock_cmd(Periph_t apb1_periphs, State_t new_state)
			{
				RCC_APB1PeriphClockCmd(apb1_periphs, new_state);
			}

			static inline constexpr void
			enable(auto&&... apb1_periphs)
			{
				clock_cmd((... | apb1_periphs), ENABLE);
			}
		};

		struct APB2
		{
			static inline constexpr void
			clock_cmd(Periph_t apb2_periphs, State_t new_state)
			{
				RCC_APB2PeriphClockCmd(apb2_periphs, new_state);
			}

			static inline constexpr void
			enable(auto&&... apb2_periphs)
			{
				clock_cmd((... | apb2_periphs), ENABLE);
			}
		};
	};
}

#endif
