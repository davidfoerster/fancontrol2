/*
 * yaml_utils.hpp
 *
 *  Created on: 14.10.2012
 *      Author: malte
 */

#pragma once
#ifndef YAML_UTILS_HPP_
#define YAML_UTILS_HPP_


namespace YAML {
	class Iterator;
}

namespace std {

	template <typename>
	struct iterator_traits;

	template<>
	struct iterator_traits< ::YAML::Iterator > {
		typedef ::ssize_t difference_type;
		typedef ::YAML::Node value_type;
		typedef value_type* pointer;
		typedef value_type& reference;
		typedef bidirectional_iterator_tag iterator_category;
	};

} // namespace std
#endif /* YAML_UTILS_HPP_ */
