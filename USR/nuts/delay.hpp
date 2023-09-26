#ifndef _NUTS_DELAY_
#define _NUTS_DELAY_

#include "stm32f4xx.h"

namespace nuts
{
#ifdef __cplusplus
	extern "C" {
#endif

	inline void
	portable_delay(volatile uint32_t n)
	{
		while (n--) {
			asm volatile("");
		}
	}

	inline void
	delay(const uint32_t n, const uint32_t unit = 2000)
	{
		portable_delay(n * unit);
	}

	void systick_delay_ms(const uint32_t); // Don't use it

#ifdef __cplusplus
	}
#endif

}

#endif