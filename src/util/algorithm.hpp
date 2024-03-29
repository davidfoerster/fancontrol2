/*
 * algorithm.hpp
 *
 *  Created on: 25.12.2011
 *      Author: malte
 */

#pragma once
#ifndef UTIL_ALGORITHM_HPP_
#define UTIL_ALGORITHM_HPP_

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/size.hpp>
#include <boost/integer/static_log2.hpp>
#include <functional>
#include <algorithm>
#include <utility>
#include <cstring>
#include <cstddef>
#include <cstdint>


#if defined BOOST_ASSERT
#	define UTIL_ASSERT(a) BOOST_ASSERT(a)
#elif defined assert
#	define UTIL_ASSERT(a) assert(a)
#else
#	define UTIL_ASSERT(a) (static_cast<void>(0))
#endif


namespace std {
	template<class, std::size_t> class array;
}


namespace util {

template <typename T>
T &clip(T &x, T &min, T &max);


template <typename T>
int signum(T &x);


template <typename T>
int compare(const T &a, const T &b);

template <typename T, typename LessThan>
int compare(const T &a, const T &b, LessThan pred);


template <typename T>
T &zero(T &);

template <typename T>
T *zero(T *x);

template <typename T, std::size_t Size>
T (&zero(T (&a)[Size]))[Size];


template <typename R1, typename R2 = R1>
bool has_prefix(const R1 &range, const R2 &prefix);

template <typename R1, typename R2, typename Predicate>
bool has_prefix(const R1 &range, const R2 &prefix, Predicate pred);


template <typename Range, typename Predicate>
bool any_of( const Range &r, Predicate pred );


template <typename Iterator, typename T>
bool any_of_equal( Iterator begin, Iterator end, const T &val );

template <typename Range, typename T>
bool any_of_equal( const Range &r, const T &val );


template <typename Range, typename Predicate>
bool all_of( const Range &r, Predicate pred );


template <typename Iterator, typename T>
bool all_of_equal( Iterator begin, Iterator end, const T &val );

template <typename Range, typename T>
bool all_of_equal( const Range &r, const T &val );


template <typename T>
void swap( T (&a)[2] );

template <typename T>
void swap( std::pair<T, T> &a );

template <typename T>
void swap( std::array<T, 2> &a );

template <typename T>
void swap( std::tuple<T, T> &a );


template <typename T1, typename T2 = T1> struct equal_to;

template <typename T1, typename T2 = T1> struct not_equal_to;


template <int N, typename Integer>
constexpr Integer shift_left(Integer x);


template <typename SourceType, SourceType Source, typename DestType, DestType Dest>
constexpr DestType convert_flagbit(SourceType src);


template <typename Integer>
constexpr Integer divide_ceil( Integer numerator, Integer denominator );


template <typename T>
constexpr inline typename std::remove_reference<T>::type
deref( T value ) { return value; }

}



// implementations ========================================

namespace util {

template <typename T>
inline T &clip(T &x, T &min, T &max)
{
	UTIL_ASSERT(min <= max);
	return std::min(std::max(x, min), max);
}


template <typename T>
inline int signum(const T &x)
{
	return compare(x, static_cast<T>(0));
}


template <typename T>
inline int compare(const T &a, const T &b)
{
	return (b < a) - (a < b);
}

template <typename T, typename LessThan>
inline int compare(const T &a, const T &b, LessThan pred)
{
	return pred(b, a) - pred(a, b);
}


template <typename T>
inline T &zero(T &x)
{
	return *zero(&x);
}

template <typename T>
inline T *zero(T *x)
{
	std::memset(&x, 0, sizeof(*x));
	return x;
}

template <typename T, std::size_t Size>
inline T (&zero(T (&a)[Size]))[Size]
{
	std::memset(a, 0, sizeof(T) * Size);
	return a;
}


template <typename R1, typename R2>
bool has_prefix(const R1 &range, const R2 &prefix)
{
	using boost::size;
	return size(range) >= size(prefix) &&
		std::equal(begin(prefix), end(prefix), begin(range));
}

template <typename R1, typename R2, typename Predicate>
bool has_prefix(const R1 &range, const R2 &prefix, Predicate pred)
{
	using boost::size;
	return size(range) >= size(prefix) &&
		std::equal(begin(prefix), end(prefix), begin(range), pred);
}


template <typename Range, typename Predicate>
inline bool any_of( const Range &r, Predicate pred )
{
	return std::any_of( boost::begin(r), boost::end(r), pred );
}


template <typename Iterator, typename T>
inline bool any_of_equal( Iterator begin, Iterator end, const T &val )
{
	return std::find( begin, end, val ) != end;
}

template <typename Range, typename T>
inline bool any_of_equal( const Range &r, const T &val )
{
	return any_of_equal( boost::begin(r), boost::end(r), val );
}


template <typename Range, typename Predicate>
inline bool all_of( const Range &r, Predicate pred )
{
	return std::all_of( boost::begin(r), boost::end(r), pred );
}


template <typename Iterator, typename T>
inline bool all_of_equal( Iterator begin, Iterator end, const T &val )
{
	typedef typename std::iterator_traits<Iterator>::value_type value_type;
	return std::all_of( begin, end, std::bind2nd(equal_to<value_type, T>(), val) );
}

template <typename Range, typename T>
inline bool all_of_equal( const Range &r, const T &val )
{
	return all_of_equal( boost::begin(r), boost::end(r), val );
}


template <typename T>
inline void swap( T (&a)[2] )
{
	std::swap( a[0], a[1] );
}

template <typename T>
inline void swap( std::pair<T, T> &a )
{
	std::swap( a.first, a.second );
}

template <typename T>
inline void swap( std::array<T, 2> &a )
{
	std::swap( a[0], a[1] );
}

template <typename T>
inline void swap( std::tuple<T, T> &a )
{
	std::swap( std::get<0>(a), std::get<1>(a) );
}


template <typename T1, typename T2>
struct equal_to
	: std::binary_function<const T1&, const T2&, bool>
{
	inline bool operator()(const T1 &a, const T2 &b) const {
		return a == b;
	}
};


template <typename T1, typename T2>
struct not_equal_to
	: std::binary_function<const T1, const T2, bool>
{
	inline bool operator()(const T1 &a, const T2 &b) const {
		return !(a == b);
	}
};


template <int N, typename Integer>
inline constexpr Integer shift_left(Integer x)
{
	return ( N >= 0 ) ?
		x << N :
		static_cast<Integer>(static_cast<typename std::make_unsigned<Integer>::type>(x) >> -N);
}

template <typename SourceType, SourceType Source, typename DestType, DestType Dest>
inline constexpr DestType convert_flagbit(SourceType src)
{
	using boost::static_log2;
	return shift_left<
				static_log2<Dest>::value - static_log2<Source>::value,
				DestType
			>(src & Source);
}


template <typename Integer>
inline constexpr Integer divide_ceil( Integer numerator, Integer denominator )
{
	return (numerator != 0) ? (numerator - 1) / denominator + 1 : 0;
}

}

#endif /* UTIL_ALGORITHM_HPP_ */
