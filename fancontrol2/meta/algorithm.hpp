/*
 * algorithm.hpp
 *
 *  Created on: 25.12.2011
 *      Author: malte
 */

#pragma once
#ifndef META_ALGORITHM_HPP_
#define META_ALGORITHM_HPP_

#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/make_unsigned.hpp>

#include <algorithm>

#if defined BOOST_ASSERT
#	define META_ASSERT(a) BOOST_ASSERT(a)
#elif defined assert
#	define META_ASSERT(a) assert(a)
#else
#	define META_ASSERT(a) (static_cast<void>(0))
#endif

namespace meta {

template <typename T>
T &clip(T &x, T &min, T &max);

template <typename T>
int signum(T &x);

template <typename T>
int compare(const T &a, const T &b);

template <typename T>
typename boost::disable_if<boost::is_integral<T>, bool>::type
in_range(const T &x, const T &min, const T &max);


template<typename Comparator, typename InputIterator, typename T>
InputIterator find(InputIterator begin, const InputIterator &end, const T& val);


template<typename Tester, typename InputIterator>
bool for_all(InputIterator begin, const InputIterator &end);


template<typename Tester, typename InputIterator, typename T>
bool exists(InputIterator begin, const InputIterator &end);



// implementations ========================================

template <typename T>
T &clip(T &x, T &min, T &max)
{
	META_ASSERT(min <= max);
	return std::min(std::max(x, min), max);
}

template <typename T>
T clip(T x, T min, T max)
{
	META_ASSERT(min <= max);
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


template <typename T>
inline typename boost::disable_if<boost::is_integral<T>, bool>::type
in_range(const T &x, const T &min, const T &max) {
	return x >= min && x < max;
}

template <typename T>
inline typename boost::enable_if<boost::is_integral<T>, bool>::type
in_range(const T x, const T min, const T max)
{
	typedef typename boost::make_unsigned<T>::type unsigned_t;
	return static_cast<unsigned_t>(x - min) < static_cast<unsigned_t>(max - min);
}

template <typename T>
inline bool in_range(const T *x, const T *min, const T *max)
{
	return (x - min) < (max - min);
}


template<typename Comparator, typename InputIterator, typename T>
InputIterator find(InputIterator begin, const InputIterator &end, const T& val)
{
	while (begin != end && Comparator::compare(*begin, val) != 0)
		++begin;
	return begin;
}


template<typename Tester, typename InputIterator>
bool for_all(InputIterator begin, const InputIterator &end)
{
	for (; begin != end; ++begin) {
		if (!Tester::test(*begin))
			return false;
	}
	return true;
}


template<typename Tester, typename InputIterator, typename T>
bool exists(InputIterator begin, const InputIterator &end)
{
	for (; begin != end; ++begin) {
		if (Tester::test(*begin))
			return true;
	}
	return false;
}

};

#endif /* META_ALGORITHM_HPP_ */
