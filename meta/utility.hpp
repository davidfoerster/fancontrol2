/*
 * utility.hpp
 *
 *  Created on: 28.01.2012
 *      Author: malte
 */

#pragma once
#ifndef UTILITY_HPP_
#define UTILITY_HPP_


#ifndef META_LOG_LEVEL
#	define META_LOG_LEVEL 0
#endif

#if META_LOG_LEVEL > 0
#	include <iostream>
#	define META_LOG(level, what) (((level) <= META_LOG_LEVEL) ? ((::std::clog << what) << ::std::endl, static_cast<void>(0)) : static_cast<void>(0))
#else
#	define META_LOG(level, what) (static_cast<void>(0))
#endif


#include <cstddef>
#include <cstring>

namespace std {
	template <typename> class auto_ptr;
}

namespace meta {


#define META_UNUSED(u) ((void)(u))

#define _META_STRING2(x) #x
#define META_STRING(x) _META_STRING2(x)

#ifdef NDEBUG
#	define META_DEBUG(x) static_cast<void>(0)
#else
#	define META_DEBUG(x) (x)
#endif


template <typename T, size_t Size>
size_t elementsof(const T (&)[Size]);


template <typename T>
T &zero(T &);


// implementations ========================================

template <typename T, size_t Size>
inline size_t elementsof(const T (&)[Size])
{
	return Size;
}


template <typename T>
inline T &zero(T &x)
{
	return *static_cast<T*>(std::memset(&x, 0, sizeof(T)));
}

}


template <typename T>
bool operator!(const std::auto_ptr<T> &p)
{
	return !p.get();
}

#endif /* UTILITY_HPP_ */
