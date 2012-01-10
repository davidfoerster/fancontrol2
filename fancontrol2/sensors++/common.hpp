/*
 * common.hpp
 *
 *  Created on: 03.01.2012
 *      Author: malte
 */

#pragma once
#ifndef SENSORS_COMMON_HPP_
#define SENSORS_COMMON_HPP_

#include <chromium/base/string_piece.h>

namespace sensors {


template <typename T, size_t Size>
inline size_t elementsof(const T (&)[Size])
{
	return Size;
}


typedef chromium::base::StringPiece string_ref;

template <string_ref::size_type Size>
inline string_ref STRING_REF(const string_ref::value_type (&s)[Size])
{
	return string_ref::make_const(s);
}


inline string_ref STRING_REF_NULL()
{
	return string_ref(NULL, 0);
}

}


#endif /* SENSORS_COMMON_HPP_ */
