/*
 * strcat.cpp
 *
 *  Created on: 05.02.2012
 *      Author: malte
 */

#include "strcat.hpp"


namespace util {

	template class basic_strcat<char>;

}


template ::std::string &operator<<(::std::string&, signed int);
template ::std::string &operator<<(::std::string&, unsigned int);
template ::std::string &operator<<(::std::string&, signed long long);
template ::std::string &operator<<(::std::string&, unsigned long long);
