/*
 * self_referenced.cpp
 *
 *  Created on: 28.01.2012
 *      Author: malte
 */

#include "self_referenced.hpp"
#include <iostream>


namespace meta {

	namespace internal {

void _self_reference_warning()
{
	std::cerr << "You should only instantiate a type derived from 'meta::self_reference' through its factory." << std::endl;
}

	}
}

