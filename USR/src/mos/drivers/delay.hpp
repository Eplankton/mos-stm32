#ifndef _MOS_DRIVER_DELAY_
#define _MOS_DRIVER_DELAY_

#include "../config.h"

namespace MOS::Driver
{
	inline void
	portable_delay(volatile uint32_t n)
	{
		while (n--) {
			asm volatile("");
		}
	}

	__attribute__((always_inline)) inline void
	delay(const uint32_t n, const uint32_t unit = 1000)
	{
		portable_delay(n * unit);
	}

	void systick_delay_ms(const uint32_t);
}

#endif