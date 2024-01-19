#ifndef _MOS_UTIL_
#define _MOS_UTIL_

#include <stdint.h>
#include <stddef.h>

#include "../config.h"
#include "../arch/cpu.hpp"

#define MOS_NO_INLINE __attribute__((noinline))
#define MOS_INLINE    __attribute__((always_inline))

#ifdef MOS_CONF_PRINTF
#include "printf.h"
#define kprintf(format, ...) printf_(format, ##__VA_ARGS__)
#define MOS_MSG(format, ...) kprintf("[MOS]: " format, ##__VA_ARGS__)
#else
#define kprintf(format, ...) ((void) 0)
#define MOS_MSG(format, ...) ((void) 0)
#endif

#ifdef MOS_CONF_ASSERT
#define MOS_ASSERT(expr, format, ...) \
	((expr) ? ((void) 0) : mos_assert_failed((uint8_t*) __FILE__, __LINE__, format))

static inline void
mos_assert_failed(uint8_t* file, uint32_t line, const char* msg)
{
	MOS_MSG("%s, %d: %s\n", file, line, msg);
	while (true) {
		asm volatile("");
	}
}

#else
#define MOS_ASSERT(expr, format, ...) ((void) 0)
#endif

namespace MOS::Utils
{
	MOS_INLINE inline void
	portable_delay(volatile uint32_t n)
	{
		while (n--) {
			asm volatile("");
		}
	}

	MOS_INLINE inline void
	delay(const uint32_t n, const uint32_t unit = 2000)
	{
		portable_delay(n * unit);
	}

	MOS_INLINE inline bool
	test_irq() { return MOS_TEST_IRQ(); }

	MOS_INLINE inline size_t
	strlen(const char* str) noexcept
	{
		const char* s = str;
		while (*s)
			++s;
		return s - str;
	}

	inline int
	strcmp(const char* str1, const char* str2) noexcept
	{
		while (*str1 && (*str1 == *str2)) {
			str1++;
			str2++;
		}
		return *(uint8_t*) str1 - *(uint8_t*) str2;
	}

	inline int
	strncmp(const char* str1, const char* str2, size_t n) noexcept
	{
		for (size_t i = 0; i < n; i++) {
			if (str1[i] != str2[i]) {
				return (uint8_t) str1[i] - (uint8_t) str2[i];
			}
			if (str1[i] == '\0') {
				break;
			}
		}
		return 0;
	}

	inline void*
	memcpy(void* dest, const void* src, size_t n)
	{
		for (size_t i = 0; i < n; i++) {
			((char*) dest)[i] = ((const char*) src)[i];
		}
		return dest;
	}

	inline void*
	memset(void* ptr, int32_t value, size_t n)
	{
		uint8_t* raw = reinterpret_cast<uint8_t*>(ptr);
		for (size_t i = 0; i < n; i++) {
			raw[i] = static_cast<uint8_t>(value);
		}
		return ptr;
	}

	// Create global critical section
	struct DisIntrGuard_t
	{
		MOS_INLINE inline DisIntrGuard_t() { MOS_DISABLE_IRQ(); }
		MOS_INLINE inline ~DisIntrGuard_t() { MOS_ENABLE_IRQ(); }
	};
}

// placement new
MOS_INLINE inline void*
operator new(size_t, void* addr) noexcept { return addr; }

#endif