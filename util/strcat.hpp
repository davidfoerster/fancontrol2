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
#include <iosfwd>
#include <cstdlib>


#ifndef UTIL_STRCAT_API
#	define UTIL_STRCAT_API extern
#endif


namespace std {

	template <typename CharT, class Traits, class Alloc, typename IntT>
	basic_string<CharT, Traits, Alloc> &operator<<(basic_string<CharT, Traits, Alloc> &dst, IntT n);

}



// implementations ============================================================

namespace std {

	template <typename CharT, class Traits, class Alloc, typename IntT>
	basic_string<CharT, Traits, Alloc> &operator<<(basic_string<CharT, Traits, Alloc> &dst, IntT n_)
	{
		using ::boost::end;
		typedef numeric_limits<IntT> limits;
		typedef typename ::boost::make_unsigned<IntT>::type UIntT;

		// digits10 is always 1 too low; plus we need one additional char for the sign
		CharT buf[limits::digits10 + 1 + limits::is_signed];
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


	template <typename CharT, class Traits, class Alloc>
	inline basic_string<CharT, Traits, Alloc> &operator<<(basic_string<CharT, Traits, Alloc> &dst, signed short n)
	{
		return dst << static_cast<signed int>(n);
	}

	template <typename CharT, class Traits, class Alloc>
	inline basic_string<CharT, Traits, Alloc> &operator<<(basic_string<CharT, Traits, Alloc> &dst, unsigned short n)
	{
		return dst << static_cast<unsigned int>(n);
	}


	UTIL_STRCAT_API template string &operator<<(string&, signed int);
	UTIL_STRCAT_API template string &operator<<(string&, unsigned int);
	UTIL_STRCAT_API template string &operator<<(string&, signed long);
	UTIL_STRCAT_API template string &operator<<(string&, unsigned long);
	UTIL_STRCAT_API template string &operator<<(string&, signed long long);
	UTIL_STRCAT_API template string &operator<<(string&, unsigned long long);

} // namespace std

#undef UTIL_STRCAT_API
#endif /* STRCAT_HPP_ */
