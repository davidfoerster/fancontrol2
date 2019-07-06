/*
 * static_string.hpp
 *
 *  Created on: 14.10.2012
 *      Author: malte
 */

#pragma once
#ifndef STATIC_STRING_HPP_
#define STATIC_STRING_HPP_

#include "static_allocator.hpp"
#include <string>


namespace util {

template <typename CharT, std::size_t Size, class Extent = std::allocator<CharT> >
class static_basic_string
	: public statically_allocated_container_wrapper<
		std::basic_string< CharT, std::char_traits<CharT>, static_allocator<CharT, Size> >
	  >
{
	typedef static_basic_string<CharT, Size, Extent> self_t;
	typedef statically_allocated_container_wrapper<
			std::basic_string< CharT, std::char_traits<CharT>, static_allocator<CharT, Size> >
		  > base_t;

public:
	typedef std::basic_string< CharT, std::char_traits<CharT>, static_allocator<CharT, Size> > nested_type;

	inline operator std::basic_string<CharT>() const {
		return std::basic_string<CharT>(this->begin(), this->end());
	}

	template <typename T>
	inline self_t &operator=( const T &other ) {
		this->assign( other.begin(), other.end() );
		return *this;
	}

};


template < std::size_t Size >
using static_string = static_basic_string<char, Size>;

} // namespace util
#endif /* STATIC_STRING_HPP_ */
