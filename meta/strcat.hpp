/*
 * strcat.hpp
 *
 *  Created on: 26.01.2012
 *      Author: malte
 */

#pragma once
#ifndef STRCAT_HPP_
#define STRCAT_HPP_

namespace std {

template <typename, class, class> class basic_string;
typedef basic_string<char> string;

template <typename, class, class> class basic_ostringstream;
typedef basic_ostringstream<char> ostringstream;

}

namespace meta {


template <
	typename CharT,
	class Traits = typename std::basic_string<CharT>::traits_type,
	class Alloc = typename std::basic_ostringstream<CharT, Traits>::allocator_type
>
struct basic_strcat
{
	typedef basic_strcat<CharT, Traits, Alloc> this_type;
	typedef std::basic_ostringstream<CharT, Traits, Alloc> stream_type;
	typedef std::basic_string<CharT, Traits, Alloc> string_type;

	stream_type stream;

	template <typename T>
	this_type &operator<<(const T &x) {
		stream << x;
		return *this;
	}

	string_type str() const {
		return stream.str();
	}

	void str(string_type &s) {
		stream.str(s);
	}

	string_type operator()() const {
		return str();
	}
};


typedef basic_strcat<char> strcat;

typedef basic_strcat<wchar_t> wstrcat;

}

#endif /* STRCAT_HPP_ */
