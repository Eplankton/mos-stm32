#ifndef _DRIVER_RCC_
#define _DRIVER_RCC_

#include "stm32f4xx.h"

namespace HAL::STM32F4xx
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

		static inline constexpr RCC_t&
		convert(Raw_t raw) { return (Self_t&) (*raw); }

		inline Raw_t get_raw() { return this; }

		static inline void
		lse_config(const uint8_t lse_status)
		{
			RCC_LSEConfig(lse_status);
		}

		static inline auto
		get_flag_status(const uint8_t flag)
		{
			return RCC_GetFlagStatus(flag);
		}

		static inline void
		rtc_clk_config(const uint32_t rtc_clk_src)
		{
			RCC_RTCCLKConfig(rtc_clk_src);
		}

		static inline void
		rtc_clk_cmd(const State_t new_state)
		{
			RCC_RTCCLKCmd(new_state);
		}

		struct AHB1
		{
			static inline void
			clock_cmd(Periph_t ahb1_periphs, State_t new_state)
			{
				RCC_AHB1PeriphClockCmd(ahb1_periphs, new_state);
			}

			static inline void
			enable(auto&&... ahb1_periphs)
			{
				clock_cmd((... | ahb1_periphs), ENABLE);
			}
		};

		struct AHB2
		{
			static inline void
			clock_cmd(Periph_t ahb2_periphs, State_t new_state)
			{
				RCC_AHB2PeriphClockCmd(ahb2_periphs, new_state);
			}

			static inline void
			enable(auto&&... ahb2_periphs)
			{
				clock_cmd((... | ahb2_periphs), ENABLE);
			}
		};

		struct AHB3
		{
			static inline void
			clock_cmd(Periph_t ahb3_periphs, State_t new_state)
			{
				RCC_AHB3PeriphClockCmd(ahb3_periphs, new_state);
			}

			static inline void
			enable(auto&&... ahb3_periphs)
			{
				clock_cmd((... | ahb3_periphs), ENABLE);
			}
		};

		struct APB1
		{
			static inline void
			clock_cmd(Periph_t apb1_periphs, State_t new_state)
			{
				RCC_APB1PeriphClockCmd(apb1_periphs, new_state);
			}

			static inline void
			enable(auto&&... apb1_periphs)
			{
				clock_cmd((... | apb1_periphs), ENABLE);
			}
		};

		struct APB2
		{
			static inline void
			clock_cmd(Periph_t apb2_periphs, State_t new_state)
			{
				RCC_APB2PeriphClockCmd(apb2_periphs, new_state);
			}

			static inline void
			enable(auto&&... apb2_periphs)
			{
				clock_cmd((... | apb2_periphs), ENABLE);
			}
		};
	};
}

#endif
