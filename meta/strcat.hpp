/*
 * strcat.hpp
 *
 *  Created on: 26.01.2012
 *      Author: malte
 */

#pragma once
#ifndef STRCAT_HPP_
#define STRCAT_HPP_

#include <string>
#include <sstream>
#include <boost/integer_traits.hpp>
#include <boost/type_traits/make_unsigned.hpp>
#include "utility.hpp"
#include "assert.hpp"
#include <cstdlib>

namespace meta {


template <
	typename CharT = char,
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


extern template class basic_strcat<char>;
typedef basic_strcat<char> strcat;

typedef basic_strcat<wchar_t> wstrcat;

}


template <typename CharT, class Traits, class Alloc, typename IntT>
std::basic_string<CharT, Traits, Alloc> &operator<<(std::basic_string<CharT, Traits, Alloc> &dst, IntT n_)
{
	using namespace meta;
	typedef std::numeric_limits<IntT> traits;
	typedef typename boost::make_unsigned<IntT>::type UIntT;

	// digits10 is always 1 too low; plus we need one additional char for the sign
	CharT buf[traits::digits10 + 1 + traits::is_signed];
	int i = static_cast<int>(elementsof(buf));
	UIntT n = (n >= 0) ? n_ : static_cast<UIntT>(-n_);

	do {
		BOOST_ASSERT(i > 0);
		buf[--i] = Traits::to_char_type(static_cast<typename Traits::int_type>(n % 10) + '0');
		n /= 10;
	} while (n != 0);

	if (n_ < 0) {
		BOOST_ASSERT(i > 0);
		buf[--i] = static_cast<CharT>('-');
	}

	return dst.append(&buf[i], elementsof(buf) - i);
}


extern template std::string &operator<<(std::string&, signed int);
extern template std::string &operator<<(std::string&, unsigned int);
extern template std::string &operator<<(std::string&, signed long long);
extern template std::string &operator<<(std::string&, unsigned long long);


#endif /* STRCAT_HPP_ */
