#pragma once
#ifndef UTIL_PREPROCESSOR_HPP_
#define UTIL_PREPROCESSOR_HPP_


#ifdef NDEBUG
#	define UTIL_DEBUG(x) (static_cast<void>(0))
#else
#	define UTIL_DEBUG(x) (x)
#endif


#endif
