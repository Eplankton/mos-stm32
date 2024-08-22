#ifndef _MOS_UTILS_
#define _MOS_UTILS_

#include <stdint.h>
#include <stddef.h>

#include "../config.h"
#include "../arch/cpu.hpp"

#define container_of(ptr, type, member) \
	(type*) ((char*) ptr - offsetof(type, member))

#define MOS_FLATTEN   __attribute__((flatten))
#define MOS_INLINE    __attribute__((always_inline))
#define MOS_NO_INLINE __attribute__((noinline))
#define MOS_NAKED     __attribute__((naked))
#define MOS_USED      __attribute__((used))
#define MOS_PACKED    __attribute__((packed))

#if (MOS_CONF_PRINTF)
#include "printf.h"
#define MOS_PUTCHAR          _putchar
#define kprintf(format, ...) printf_(format, ##__VA_ARGS__)
#define MOS_MSG(format, ...) kprintf("[MOS]: " format "\n", ##__VA_ARGS__)
#else
#define MOS_PUTCHAR          ((void) 0)
#define kprintf(format, ...) ((void) 0)
#define MOS_MSG(format, ...) ((void) 0)
#endif

#if (MOS_CONF_ASSERT)
#define MOS_ASSERT(expr, format, ...) \
	((expr) ? ((void) 0) : mos_assert_failed((uint8_t*) __FILE__, __LINE__, (uint8_t*) __func__, format))

static inline void
mos_assert_failed(void* file, uint32_t line, void* func, const char* msg)
{
	MOS_MSG(
	    "%s(%d) <%s>: \"%s\"",
	    file, line, func, msg
	);
	while (true) {
		MOS_NOP();
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
			MOS_NOP();
		}
	}

	MOS_INLINE inline void
	delay(
	    const uint32_t n,
	    const uint32_t unit = 2000
	)
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
			++str1;
			++str2;
		}
		return *(uint8_t*) str1 - *(uint8_t*) str2;
	}

	inline int
	strncmp(
	    const char* str1, const char* str2, size_t n
	) noexcept
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
			((uint8_t*) dest)[i] = ((const uint8_t*) src)[i];
		}
		return dest;
	}

	inline void*
	memset(void* ptr, uint8_t value, size_t n)
	{
		auto raw = (uint8_t*) ptr;
		for (size_t i = 0; i < n; i++) {
			raw[i] = value;
		}
		return ptr;
	}

	struct Range
	{
		using Raw_t = int32_t;
		const Raw_t st, ed, n;

		MOS_INLINE
		inline Range(
		    Raw_t _st, Raw_t _ed, Raw_t _n = 1
		): st(_st), ed(_ed), n(_n) {}

		struct Iter_t
		{
			const Raw_t step;
			Raw_t raw;

			MOS_INLINE
			inline Iter_t(Raw_t _raw, Raw_t _step)
			    : raw(_raw), step(_step) {}

			MOS_INLINE
			inline void operator++() { raw += step; }

			MOS_INLINE inline auto&
			operator*() const { return raw; }

			MOS_INLINE inline bool
			operator!=(const Iter_t& other) const
			{
				return step > 0 ? raw < *other
				                : raw > *other;
			}
		};

		MOS_INLINE inline Iter_t
		begin() const { return {st, n}; }

		MOS_INLINE inline Iter_t
		end() const { return {ed, n}; }

		MOS_INLINE inline Iter_t
		rbegin() const { return {ed - n, -n}; }

		MOS_INLINE inline Iter_t
		rend() const { return {st - n, -n}; }
	};

	// Enter/Exit Global Critical Section
	struct IntrGuard_t
	{
		using NestCnt_t = volatile int32_t;

		MOS_INLINE
		inline IntrGuard_t()
		{
			MOS_DISABLE_IRQ();
			cnt += 1;
		}

		MOS_INLINE
		inline ~IntrGuard_t()
		{
			cnt -= 1;
			if (cnt <= 0) {
				MOS_ENABLE_IRQ();
			}
		}

	private:
		static inline NestCnt_t cnt = 0;
	};

	template <typename T>
	inline constexpr T&&
	move(T& x) noexcept
	{
		return static_cast<T&&>(x);
	}

	template <typename T>
	inline constexpr T&&
	move(T&& x) noexcept
	{
		return static_cast<T&&>(x);
	}
}

// Inplace new
MOS_INLINE inline void*
operator new(size_t, void* addr) noexcept { return addr; }

MOS_INLINE inline constexpr uint32_t
operator"" _ms(uint64_t raw) noexcept
{
	return raw * MOS_CONF_SYSTICK / 1000;
}

MOS_INLINE inline constexpr uint32_t
operator"" _s(uint64_t raw) noexcept
{
	return raw * MOS_CONF_SYSTICK;
}

#endif