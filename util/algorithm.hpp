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
#include <functional>
#include <algorithm>
#include <utility>
#include <cstring>
#include <cstddef>


#if defined BOOST_ASSERT
#	define UTIL_ASSERT(a) BOOST_ASSERT(a)
#elif defined assert
#	define UTIL_ASSERT(a) assert(a)
#else
#	define UTIL_ASSERT(a) (static_cast<void>(0))
#endif


namespace std {
	template<class, ::std::size_t> class array;
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

	template <typename T, ::std::size_t Size>
	T (&zero(T (&a)[Size]))[Size];


	template <typename R>
	bool has_prefix(const R &range, const R &prefix);

	template <typename R1, typename R2>
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
	void swap( ::std::pair<T, T> &a );

	template <typename T>
	void swap( ::std::array<T, 2> &a );

	template <typename T>
	void swap( ::std::tuple<T, T> &a );


	template <typename T1, typename T2 = T1> struct equal_to;

	template <typename T1, typename T2 = T1> struct not_equal_to;

}



// implementations ========================================

namespace util {

	template <typename T>
	inline T &clip(T &x, T &min, T &max)
	{
	    UTIL_ASSERT(min <= max);
	    return ::std::min(::std::max(x, min), max);
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
	    ::std::memset(&x, 0, sizeof(*x));
	    return x;
	}

	template <typename T, ::std::size_t Size>
	inline T (&zero(T (&a)[Size]))[Size]
	{
	    ::std::memset(a, 0, sizeof(a));
	    return a;
	}


	template <typename R>
	inline bool has_prefix(const R &range, const R &prefix)
	{
	    return has_prefix<R, R>(range, prefix);
	}


	template <typename R1, typename R2>
	bool has_prefix(const R1 &range, const R2 &prefix)
	{
	    using ::boost::size;
	    return size(range) >= size(prefix) &&
		    ::std::equal(begin(prefix), end(prefix), begin(range));
	}

	template <typename R1, typename R2, typename Predicate>
	bool has_prefix(const R1 &range, const R2 &prefix, Predicate pred)
	{
	    using ::boost::begin;
	    using ::boost::end;
	    using ::boost::size;
	    return size(range) >= size(prefix) &&
		    ::std::equal(begin(prefix), end(prefix), begin(range), pred);
	}


	template <typename Range, typename Predicate>
	inline bool any_of( const Range &r, Predicate pred )
	{
	    return ::std::any_of( ::boost::begin(r), ::boost::end(r), pred );
	}


	template <typename Iterator, typename T>
	inline bool any_of_equal( Iterator begin, Iterator end, const T &val )
	{
	    return ::std::find( begin, end, val ) != end;
	}

	template <typename Range, typename T>
	inline bool any_of_equal( const Range &r, const T &val )
	{
	    return any_of_equal( ::boost::begin(r), ::boost::end(r), val );
	}


	template <typename Range, typename Predicate>
	inline bool all_of( const Range &r, Predicate pred )
	{
	    return ::std::all_of( ::boost::begin(r), ::boost::end(r), pred );
	}


	template <typename Iterator, typename T>
	inline bool all_of_equal( Iterator begin, Iterator end, const T &val )
	{
	    typedef typename ::std::iterator_traits<Iterator>::value_type value_type;
	    return ::std::all_of( begin, end, ::std::bind2nd(equal_to<value_type, T>(), val) );
	}

	template <typename Range, typename T>
	inline bool all_of_equal( const Range &r, const T &val )
	{
	    return all_of_equal( ::boost::begin(r), ::boost::end(r), val );
	}


	template <typename T>
	inline void swap( T (&a)[2] )
	{
	    ::std::swap( a[0], a[1] );
	}

	template <typename T>
	inline void swap( ::std::pair<T, T> &a )
	{
	    ::std::swap( a.first, a.second );
	}

	template <typename T>
	inline void swap( ::std::array<T, 2> &a )
	{
	    ::std::swap( a[0], a[1] );
	}

	template <typename T>
	inline void swap( ::std::tuple<T, T> &a )
	{
	    ::std::swap( ::std::get<0>(a), ::std::get<1>(a) );
	}


	template <typename T1, typename T2>
	struct equal_to
	    : ::std::binary_function<const T1&, const T2&, bool>
	{
	    inline bool operator()(const T1 &a, const T2 &b) const {
		    return a == b;
	    }
	};


	template <typename T1, typename T2>
	struct not_equal_to
	    : ::std::binary_function<const T1, const T2, bool>
	{
	    inline bool operator()(const T1 &a, const T2 &b) const {
		    return !(a == b);
	    }
	};

}

#endif /* UTIL_ALGORITHM_HPP_ */
