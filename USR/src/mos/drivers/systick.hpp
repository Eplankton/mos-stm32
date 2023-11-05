#ifndef _MOS_DRIVER_SYSTICK_
#define _MOS_DRIVER_SYSTICK_

#include "../config.h"

namespace MOS::Driver
{
	struct SysTick_t : public SysTick_Type
	{
		// Type alias
		using Self_t = SysTick_t;
		using Raw_t  = SysTick_Type*;
		using Freq_t = const uint32_t;
		using Time_t = const uint32_t;

		SysTick_t()                  = delete;
		SysTick_t(const Self_t& src) = delete;

		// Functions
		static constexpr inline SysTick_t&
		convert(Raw_t raw) { return (Self_t&) (*raw); }

		static inline void
		config(Freq_t freq = 1000)
		{
			// SystemFrequency / 1000    every 1ms
			// SystemFrequency / 10000   every 100us
			// SystemFrequency / 100000  every 10us
			// SystemFrequency / 1000000 every 1us

			SysTick_Config(SystemCoreClock / freq);
		}
	};
}

#endif