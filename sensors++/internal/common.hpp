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


typedef chromium::base::StringPiece string_ref;

string_ref STRING_REF();

template <string_ref::size_type Size>
string_ref STRING_REF(const string_ref::value_type (&s)[Size]);


template <class InputIterator>
InputIterator find_prefix(InputIterator begin, const InputIterator &end, const string_ref &s);



// implementations ========================================

template <string_ref::size_type Size>
inline string_ref STRING_REF(const string_ref::value_type (&s)[Size])
{
	return string_ref::make_const(s);
}


inline
string_ref STRING_REF()
{
	return string_ref(0, 0);
}


template <class InputIterator>
InputIterator find_prefix(InputIterator begin, const InputIterator &end, const string_ref &s)
{
	for (; begin != end; ++begin) {
		if (s.starts_with(*begin))
			return begin;
	}
	return end;
}

}


#endif /* SENSORS_COMMON_HPP_ */
