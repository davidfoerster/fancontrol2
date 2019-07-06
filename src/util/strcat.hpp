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
#include <string>
#include <array>
#include <limits>
#include <cstdint>


#ifndef UTIL_STRCAT_API
#	define UTIL_STRCAT_API extern
#endif


namespace std {

template <typename CharT, class Traits, class Alloc, typename IntT>
basic_string<CharT, Traits, Alloc> &operator<<(basic_string<CharT, Traits, Alloc> &dst, IntT n);

}



// implementations ============================================================


namespace util { namespace detail {

template <typename CharT, class Traits, class Alloc>
std::basic_string<CharT, Traits, Alloc> &__strcat_impl(
		std::basic_string<CharT, Traits, Alloc> &dst,
		std::uintmax_t n,
		bool negative
) {
	// digits10 is always 1 too low
	std::array<CharT, std::numeric_limits<std::uintmax_t>::digits10 + 1> buf;
	auto p = buf.end();

	do {
		BOOST_ASSERT(p != buf.begin());
		*(--p) = Traits::to_char_type(static_cast<typename Traits::int_type>(n % 10) + '0');
		n /= 10;
	} while (n != 0);

	if (negative) {
		dst.reserve(buf.end() - p + 1);
		dst.push_back('-');
	}

	return dst.append(p, buf.end() - p);
}


UTIL_STRCAT_API template std::string &__strcat_impl(std::string&, std::uintmax_t, bool);
UTIL_STRCAT_API template std::wstring &__strcat_impl(std::wstring&, std::uintmax_t, bool);

} /* namespace detail */ } /* namespace util */


namespace std {

template <typename CharT, class Traits, class Alloc, typename IntT>
inline basic_string<CharT, Traits, Alloc> &operator<<(basic_string<CharT, Traits, Alloc> &dst, IntT n)
{
	const bool negative = n < 0;
	if (negative) n = -n;
	return ::util::detail::__strcat_impl(dst, static_cast<uintmax_t>(n), negative);
}

} // namespace std

#undef UTIL_STRCAT_API
#endif /* STRCAT_HPP_ */
