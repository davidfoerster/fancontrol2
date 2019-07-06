/*
 * yaml_utils.hpp
 *
 *  Created on: 14.10.2012
 *      Author: malte
 */

#pragma once
#ifndef UTIL_YAML_HPP_
#define UTIL_YAML_HPP_
#include <yaml-cpp/yaml.h>

// implementations ========================================

namespace YAML {

	template<class T>
	inline T& operator>>(const Node &node, T &field) {
		field = node.as<T>();
		return field;
	}

} // namespace YAML

#endif /* UTIL_YAML_HPP_ */
