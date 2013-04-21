/*
 * common.cpp
 *
 *  Created on: 15.10.2012
 *      Author: malte
 */

#include "common.hpp"
#include "../../util/in_range.hpp"
#include <cstring>


namespace sensors {

	bool starts_with_nonzero_digit(const string_ref &s)
	{
		return !s.empty() && util::in_range<char>(s.front(), '1', '9' + 1);
	}

} /// namespace sensrors
