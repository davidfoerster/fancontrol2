/*
 * in_range.hpp
 *
 *  Created on: 25.09.2012
 *      Author: malte
 */

#pragma once
#ifndef UTIL_IN_RANGE_HPP_
#define UTIL_IN_RANGE_HPP_

#include "enable_if.hpp"
#include <cstdint>


namespace util {

template <typename T>
typename std::_disable_if< std::is_integral<T>, bool>::type
in_range(const T &x, const T &min, const T &max);

template <typename T>
typename std::_enable_if< std::is_integral<T>, bool>::type
in_range(const T x, const T min, const T max);

template <typename T>
bool in_range(const T *x, const T *min, const T *max);



// implementations ========================================

template <typename T>
inline
typename std::_disable_if< std::is_integral<T>, bool>::type
in_range(const T &x, const T &min, const T &max)
{
	UTIL_ASSERT(min <= max);
	return x >= min && x < max;
}

template <typename T>
inline
typename std::_enable_if< std::is_integral<T>, bool>::type
in_range(const T x, const T min, const T max)
{
	UTIL_ASSERT(min <= max);
	typedef typename std::make_unsigned<T>::type unsigned_t;
	return static_cast<unsigned_t>(x - min) < static_cast<unsigned_t>(max - min);
}

template <typename T>
inline bool in_range(const T *x, const T *min, const T *max)
{
	return in_range(
			reinterpret_cast< ::uintptr_t>(x),
			reinterpret_cast< ::uintptr_t>(min),
			reinterpret_cast< ::uintptr_t>(max));
}

} // namespace util
#endif /* UTIL_IN_RANGE_HPP_ */
