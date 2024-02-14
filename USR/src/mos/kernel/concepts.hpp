#ifndef _MOS_CONCEPTS_
#define _MOS_CONCEPTS_

// Since C++20.

#include "type.hpp"

namespace MOS::Concepts
{
	// clang-format off

	template <typename T, typename U>
	struct is_same
	{
		static constexpr bool value = false;
	};

	template <typename T>
	struct is_same<T, T>
	{
		static constexpr bool value = true;
	};

	template <typename T, typename U>
	constexpr bool is_same_v = is_same<T, U>::value;

	template <typename T, typename U>
	concept Same = is_same_v<T, U>;

	template <typename Fn, typename Ret, typename... Argv>
	concept Invocable = requires(Fn fn, Argv... argv)
	{
		{ fn(argv...) } -> Same<Ret>;
	};

	template <typename Box>
	concept Iterable = requires(Box x)
	{
		typename Box::value_type;
		typename Box::iterator;

		x.begin();
		x.end();
	};

	template <typename Box>
	concept Container = Iterable<Box> &&
	        requires(Box x)
	{
		x.size();
		x.front();
		x.back();
	};

	template <typename T, typename U = T>
	concept Add = requires(T x, U y)
	{
		x + y;
	};

	template <typename T, typename U = T>
	concept Minus = requires(T x, U y)
	{
		x - y;
	};

	template <typename T, typename U = T>
	concept Multi = requires(T x, U y)
	{
		x * y;
	};

	template <typename T, typename U = T>
	concept Div = requires(T x, U y)
	{
		x / y;
	};

	template <typename T, typename U = T>
	concept Less = requires(T x, U y)
	{
		x < y;
	};

	template <typename T, typename U = T>
	concept Greater = requires(T x, U y)
	{
		x > y;
	};

	template <typename T, typename U = T>
	concept Equal = requires(T x, U y)
	{
		x == y;
	};

	template <typename T, typename U = T>
	concept Partial_Order = Less<T, U> && Greater<T, U>;

	template <typename T, typename U = T>
	concept Order = Partial_Order<T, U> && Equal<T, U>;

	template <typename T, typename U = T>
	concept Arithmetic =
	        Add  <T, U> &&
	        Minus<T, U> &&
	        Multi<T, U> &&
	        Div  <T, U>;

	template <typename>
	struct is_pointer
	{
		static constexpr bool value = false;
	};

	template <typename T>
	struct is_pointer<T*>
	{
		static constexpr bool value = true;
	};

	template <typename T>
	constexpr bool is_pointer_v = is_pointer<T>::value;

	template <typename Ptr>
	concept Pointer = is_pointer_v<Ptr>;

	template <typename Ptr>
	concept Valid_Pointer =
	         Pointer<Ptr> 	      &&
	        !Same<Ptr, nullptr_t> &&
	        !Same<Ptr, void*>;

	template <typename Iter>
	concept ForwardIter = Valid_Pointer<Iter> 
        || requires(Iter it)
	{
		typename Iter::value_type;
		*it;
		++it;
	};

	template <typename Iter>
	concept BidirectIter =
	        ForwardIter<Iter> && requires(Iter it)
	{
		--it;
	};

	template <typename Iter>
	concept RandomIter =
	        BidirectIter<Iter> && requires(Iter x, u64 n)
	{
		x + n;
		x - n;
		x[n];
		x - x;
	};

	template <ForwardIter Iter>
	struct deref
	{
		using type = typename Iter::value_type;
	};

	template <typename T>
	struct deref<T*>
	{
		using type = T;
	};

	template <typename T>
	struct deref<T&>
	{
		using type = T;
	};

	template <typename Ref>
	using deref_t = typename deref<Ref>::type;

	template <typename>
	struct is_numeric
	{
		static constexpr bool value = false;
	};

	template <typename T>
	constexpr bool is_numeric_v = is_numeric<T>::value;

	template <>
	struct is_numeric<i8>
	{
		static constexpr bool value = true;
	};

	template <>
	struct is_numeric<i16>
	{
		static constexpr bool value = true;
	};

	template <>
	struct is_numeric<i32>
	{
		static constexpr bool value = true;
	};

	template <>
	struct is_numeric<i64>
	{
		static constexpr bool value = true;
	};

	template <>
	struct is_numeric<u8>
	{
		static constexpr bool value = true;
	};

	template <>
	struct is_numeric<u16>
	{
		static constexpr bool value = true;
	};

	template <>
	struct is_numeric<u32>
	{
		static constexpr bool value = true;
	};

	template <>
	struct is_numeric<u64>
	{
		static constexpr bool value = true;
	};

	template <>
	struct is_numeric<f32>
	{
		static constexpr bool value = true;
	};

	template <>
	struct is_numeric<f64>
	{
		static constexpr bool value = true;
	};

	template <typename T>
	concept Numeric = is_numeric_v<i8>  ||
	                  is_numeric_v<i16> ||
	                  is_numeric_v<i32> ||
	                  is_numeric_v<i64> ||
	                  is_numeric_v<u8>  ||
	                  is_numeric_v<u16> ||
	                  is_numeric_v<u32> ||
	                  is_numeric_v<u64> ||
	                  is_numeric_v<f32> ||
	                  is_numeric_v<f64>;

	// clang-format on
}

#endif