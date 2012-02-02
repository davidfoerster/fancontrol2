/*
 * utility.hpp
 *
 *  Created on: 28.01.2012
 *      Author: malte
 */

#pragma once
#ifndef UTILITY_HPP_
#define UTILITY_HPP_

#include <cstddef>
#include <cstring>

namespace std {
	template <typename> class auto_ptr;
}

namespace meta {


#define META_UNUSED(u) ((void)(u))


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
