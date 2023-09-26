#ifndef _NUTS_TYPE_
#define _NUTS_TYPE_

#include <stdint.h>

// Primitive data types
namespace nuts
{
	using i8  = int8_t;
	using i16 = int16_t;
	using i32 = int32_t;
	using i64 = int64_t;

	using u8  = uint8_t;
	using u16 = uint16_t;
	using u32 = uint32_t;
	using u64 = uint64_t;

	using f32 = float;
	using f64 = double;

	using usize = uintptr_t;
	using isize = intptr_t;

	using nullptr_t = decltype(nullptr);
}

#endif