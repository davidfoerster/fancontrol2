/*
 * static_vector.hpp
 *
 *  Created on: 14.10.2012
 *      Author: malte
 */

#pragma once
#ifndef STATIC_VECTOR_HPP_
#define STATIC_VECTOR_HPP_

#include "static_allocator.hpp"
#include <vector>


namespace util {

	template <typename T, std::size_t Size, class Extent = std::allocator<T> >
	class static_vector
		: public statically_allocated_container_wrapper<
				std::vector< T, static_allocator<T, Size, Extent> >
		  >
	{
	public:
		typedef std::vector< T, static_allocator<T, Size, Extent> > nested_type;
	};

} // namespace util
#endif /* STATIC_VECTOR_HPP_ */
