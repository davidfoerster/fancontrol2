/*
 * common.hpp
 *
 *  Created on: 03.01.2012
 *      Author: malte
 */

#pragma once
#ifndef SENSORS_COMMON_HPP_
#define SENSORS_COMMON_HPP_

#include "util/stringpiece/stringpiece.hpp"
#include "util/memory.hpp"


namespace std {
	template<typename, typename, class, class, class> class unordered_map;
}


namespace sensors {

template <class>
struct rebind_ptr;

template <typename Key, typename T, class Hash, class Equal, class Alloc>
struct rebind_ptr<
	std::unordered_map<Key, util::weak_ptr<T>, Hash, Equal, Alloc> >
{
	typedef std::unordered_map<
		Key, util::shared_ptr<T>, Hash, Equal,
		typename std::allocator_traits<Alloc>::template rebind_alloc<
			std::pair< const Key, util::shared_ptr<T> >
		>
	> other;
};


typedef util::stringpiece string_ref;

string_ref STRING_REF();

template <string_ref::size_type Size>
string_ref STRING_REF(const string_ref::value_type (&s)[Size]);


bool starts_with_nonzero_digit(const string_ref &);



// implementations ========================================

template <string_ref::size_type Size>
inline string_ref STRING_REF(const string_ref::value_type (&s)[Size])
{
	BOOST_ASSERT(string_ref::traits_type::find(s, Size, 0) == &s[Size-1]);
	return string_ref(s, s + Size - 1);
}


inline string_ref STRING_REF()
{
	return string_ref(nullptr);
}

}


#endif /* SENSORS_COMMON_HPP_ */
