/*
 * algorithm.hpp
 *
 *  Created on: 25.12.2011
 *      Author: malte
 */

#pragma once
#ifndef META_ALGORITHM_HPP_
#define META_ALGORITHM_HPP_

namespace meta {


template<typename Comparator, typename InputIterator, typename T>
inline InputIterator find(InputIterator begin, const InputIterator &end, const T& val)
{
	while (begin != end && Comparator::compare(*begin, val) != 0)
		++begin;
	return begin;
}

};

#endif /* META_ALGORITHM_HPP_ */
