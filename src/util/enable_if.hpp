/*
 * enable_if.hpp
 *
 *  Created on: 19.04.2013
 *      Author: malte
 */

#pragma once
#ifndef UTIL_ENABLE_IF_HPP_
#define UTIL_ENABLE_IF_HPP_

#include <type_traits>


namespace std {

template <class C, typename T>
struct _enable_if
	: enable_if<C::value, T>
{ };


template <bool C, typename T>
struct disable_if
	: enable_if<!C, T>
{ };


template <class C, typename T>
struct _disable_if
	: disable_if<C::value, T>
{ };

}

#endif /* UTIL_ENABLE_IF_HPP_ */
