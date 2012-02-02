/*
 * assert.hpp
 *
 *  Created on: 25.01.2012
 *      Author: malte
 */

#pragma once
#ifndef META_ASSERT_HPP_
#define META_ASSERT_HPP_

#include <boost/current_function.hpp>
#include <boost/assert.hpp>


#ifndef BOOST_ASSERT_MSG_FILE
#	include <cstdio>
#	define BOOST_ASSERT_MSG_FILE stderr
#endif


#ifdef BOOST_DISABLE_ASSERTS
#	define assert_printf(expr, format, ...) (static_cast<void>(0))
#else
#	define assert_printf(expr, format, ...) ((expr) ? static_cast<void>(0) : \
		::meta::assertion::detail::assert_printf_fail(#expr, BOOST_CURRENT_FUNCTION, __FILE__, __LINE__, BOOST_ASSERT_MSG_FILE, format, ## __VA_ARGS__))

namespace meta {
	namespace assertion {
		namespace detail {

			void assert_printf_fail(
					const char *expr,
					const char *function, const char *file, unsigned int line,
					::std::FILE *dst, const char *format, ...)
				throw() __attribute__ ((__noreturn__));

		}
	}
}

#endif


#ifdef BOOST_DISABLE_ASSERTS
#	define BOOST_VERIFY_R(expr) (!!(expr))
#	define BOOST_VERIFY_P(expr) (expr)
#else
#	if defined BOOST_ENABLE_ASSERT_HANDLER
#		define BOOST_VERIFY_R(expr) ((expr) || (::boost::assertion_failed(#expr, BOOST_CURRENT_FUNCTION, __FILE__, __LINE__), false))
#	elif defined __GNUC__
#		define BOOST_VERIFY_R(expr) ((expr) || (::__assert_fail(#expr, __FILE__, __LINE__, BOOST_CURRENT_FUNCTION), false))

#	else
#		error No definition for BOOST_VERIFY_R in this case
#	endif

#ifndef errno
#	include <errno.h>
#endif

#	define BOOST_VERIFY_P(expr) ((expr) ? static_cast<void>(0) : \
		::meta::assertion::detail::assert_perror_fail(#expr, BOOST_CURRENT_FUNCTION, __FILE__, __LINE__, BOOST_ASSERT_MSG_FILE, errno))

namespace meta {
	namespace assertion {
		namespace detail {

			void assert_perror_fail(
					const char *expr,
					const char *function, const char *file, unsigned int line,
					::std::FILE *dst, int errnum)
				throw() __attribute__ ((__noreturn__));

		}
	}
}

#endif

#endif /* ASSERT_HPP_ */
