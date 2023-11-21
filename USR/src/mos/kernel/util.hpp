#ifndef _MOS_UTIL_
#define _MOS_UTIL_

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "../../config.h"
#include "../arch/cpu.hpp"

#ifdef MOS_CONF_PRINTF
#include "printf.h"
#define MOS_MSG(format, ...) printf(format, ##__VA_ARGS__)
#else
#define MOS_MSG(format, ...) ((void) 0)
#endif

#ifdef MOS_CONF_ASSERT
#define MOS_ASSERT(expr, format, ...) \
	((expr) ? (void) 0 : mos_assert_failed((uint8_t*) __FILE__, __LINE__, format))

inline void mos_assert_failed(uint8_t* file, uint32_t line, const char* message)
{
	MOS_MSG("%s, %d: %s\n", file, line, message);
	while (true) {
		asm volatile("");
	}
}

#else
#define MOS_ASSERT(expr, format, ...) ((void) 0)
#endif

namespace MOS::Util
{
	inline void
	portable_delay(volatile uint32_t n)
	{
		while (n--) {
			asm volatile("");
		}
	}

	__attribute__((always_inline)) inline void
	delay(const uint32_t n, const uint32_t unit = 2000)
	{
		portable_delay(n * unit);
	}

	__attribute__((always_inline)) inline bool
	test_irq() { return MOS_TEST_IRQ(); }

	struct DisIntrGuard
	{
		__attribute__((always_inline)) DisIntrGuard()
		{
			MOS_DISABLE_IRQ();
		}

		__attribute__((always_inline)) ~DisIntrGuard()
		{
			MOS_ENABLE_IRQ();
		}

		__attribute__((always_inline)) inline void
		release() { this->~DisIntrGuard(); }
	};
}

// placement new
__attribute__((always_inline)) inline void*
operator new(std::size_t, void* addr) noexcept { return addr; }

#endif