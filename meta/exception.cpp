/*
 * exception.cpp
 *
 *  Created on: 31.01.2012
 *      Author: malte
 */

#include "exception.hpp"
#include "strcat.hpp"

namespace meta {

using boost::exception_detail::get_info;


const char *exception_base::what() const throw()
{
	if (msg.empty()) {
		const std::string *what = get_info<what_t>::get(*this);
		const_cast<exception_base*>(this)->msg = *what;
	}
	return msg.c_str();
}


runtime_error::runtime_error(const std::runtime_error &e)
{
	*this << what_t(e.what());
}


const char *io_error::what() const throw()
{
	if (msg.empty()) {
		std::string &msg = const_cast<io_error*>(this)->msg;

		{
			const std::string *what = get_info<what_t>::get(*this);
			if (what && !what->empty()) {
				msg = *what;
			} else {
				msg = "I/O error";
			}
		}

		{
			const int *errnum = get_info<errno_code>::get(*this);
			if (errnum) {
				const char *errmsg = std::strerror(*errnum);
				BOOST_ASSERT(errmsg && *errmsg);
				((msg += ':') += ' ') += errmsg;
				((msg += " [errno=") << *errnum) += ']';
			}
		}

		{
			const std::string *filename = get_info<io_error::filename>::get(*this);
			if (filename) {
				((msg += " on `") += *filename) += '\'';
			}
		}
	}
	return msg.c_str();
}

}
