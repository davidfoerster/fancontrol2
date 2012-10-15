/*
 * utility.hpp
 *
 *  Created on: 28.01.2012
 *      Author: malte
 */

#pragma once
#ifndef UTIL_LOGGING_HPP_
#define UTIL_LOGGING_HPP_


#ifndef UTIL_LOG_LEVEL
#	define UTIL_LOG_LEVEL 0
#endif

#if UTIL_LOG_LEVEL > 0
#	include <iostream>

namespace util {
	namespace detail {

	    template <typename T>
	    inline void __log( const T &what )
	    {
		    (::std::clog << what) << ::std::endl;
	    }

	}
}
#	define UTIL_LOG(level, what) (((level) <= (UTIL_LOG_LEVEL)) ? ::util::detail::__log(what) : static_cast<void>(0))

#else
#	define UTIL_LOG(level, what) (static_cast<void>(0))
#endif


#define UTIL_UNUSED(u) (static_cast<void>(u))

#define _UTIL_STRING2(x) #x
#define UTIL_STRING(x) _UTIL_STRING2(x)

#ifdef NDEBUG
#	define UTIL_DEBUG(x) (static_cast<void>(0))
#else
#	define UTIL_DEBUG(x) (x)
#endif


// implementations ========================================



#endif /* UTIL_LOGGING_HPP_ */
