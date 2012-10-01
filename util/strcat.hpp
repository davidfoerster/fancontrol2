/*
 * strcat.hpp
 *
 *  Created on: 26.01.2012
 *      Author: malte
 */

#pragma once
#ifndef UTIL_STRCAT_HPP_
#define UTIL_STRCAT_HPP_

#include "assert.hpp"
#include <boost/range/size.hpp>
#include <boost/integer_traits.hpp>
#include <boost/type_traits/make_unsigned.hpp>
#include <string>
#include <sstream>
#include <cstdlib>


namespace util {


	template <
		typename CharT = char,
		class Traits = typename ::std::basic_string<CharT>::traits_type,
		class Alloc = typename ::std::basic_ostringstream<CharT, Traits>::allocator_type
	>
	struct basic_strcat
	{
		typedef basic_strcat<CharT, Traits, Alloc> this_type;
		typedef ::std::basic_ostringstream<CharT, Traits, Alloc> stream_type;
		typedef ::std::basic_string<CharT, Traits, Alloc> string_type;

		stream_type stream;

		template <typename T>
		this_type &operator<<(const T &x) {
			stream << x;
			return *this;
		}

		string_type str() const {
			return stream.str();
		}

		void str(const string_type &s) {
			stream.str(s);
		}

		inline operator string_type() const {
			return str();
		}
	};


	typedef basic_strcat<char> strcat;
	typedef basic_strcat<wchar_t> wstrcat;


	extern template class basic_strcat<char>;

}


template <typename CharT, class Traits, class Alloc, typename IntT>
::std::basic_string<CharT, Traits, Alloc> &operator<<(::std::basic_string<CharT, Traits, Alloc> &dst, IntT n_)
{
	using namespace util;
	using ::boost::end;
	typedef ::std::numeric_limits<IntT> traits;
	typedef typename ::boost::make_unsigned<IntT>::type UIntT;

	// digits10 is always 1 too low; plus we need one additional char for the sign
	CharT buf[traits::digits10 + 1 + traits::is_signed];
	CharT *p = end(buf);
	UIntT n = static_cast<UIntT>((n >= 0) ? n_ : -n_);

	do {
		BOOST_ASSERT(p > buf);
		*(--p) = Traits::to_char_type(static_cast<typename Traits::int_type>(n % 10) + '0');
		n /= 10;
	} while (n != 0);

	if (n_ < 0) {
		BOOST_ASSERT(p > buf);
		*(--p) = static_cast<CharT>('-');
	}

	return dst.append(p, end(buf) - p);
}


extern template ::std::string &operator<<(::std::string&, signed int);
extern template ::std::string &operator<<(::std::string&, unsigned int);
extern template ::std::string &operator<<(::std::string&, signed long long);
extern template ::std::string &operator<<(::std::string&, unsigned long long);

#endif /* STRCAT_HPP_ */
