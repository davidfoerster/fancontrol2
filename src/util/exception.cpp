/*
 * exception.cpp
 *
 *  Created on: 31.01.2012
 *      Author: malte
 */

#include "exception.hpp"
#include "strcat.hpp"
#include <cstring>

namespace util {

	using boost::exception_detail::get_info;


	exception_base::~exception_base() throw()
	{ }


	const char *exception_base::what() const throw()
	{
		if (msg.empty()) {
			const std::string *const what = get_info<what_t>::get(*this);
			if (what && !what->empty()) {
				msg = *what;
			} else {
				msg = "<null>";
			}
		}
		return msg.c_str();
	}


	runtime_error::runtime_error() throw()
	{ }


	runtime_error::runtime_error(const std::runtime_error &e)
	{
		*this << what_t(e.what());
	}


	runtime_error::~runtime_error() throw()
	{ }


	io_error::~io_error() throw()
	{ }


	const char *io_error::what() const throw()
	{
		if (msg.empty()) {
			{
				const std::string *const what = get_info<what_t>::get(*this);
				if (what && !what->empty()) {
					msg = *what;
				} else {
					msg = "I/O error";
				}
			}

			{
				const int *const errnum = get_info<errno_code>::get(*this);
				if (errnum) {
					const char *const errmsg = std::strerror(*errnum);
					BOOST_ASSERT(errmsg && *errmsg);
					((msg += ':') += ' ') += errmsg;
					msg += " (errno=";
					(msg << *errnum) += ')';
				}
			}

			{
				const std::string *const filename = get_info<io_error::filename>::get(*this);
				if (filename && !filename->empty()) {
					((msg += " on `") += *filename) += '\'';
				}
			}
		}
		return msg.c_str();
	}


	namespace ns_null_pointer_exception {

		null_pointer_exception::~null_pointer_exception() throw()
		{ }


		void check(const void *p, const char *var_name) throw (null_pointer_exception)
		{
			if (!p) {
				BOOST_THROW_EXCEPTION(null_pointer_exception() << null_pointer_exception::var_name(var_name));
			}
		}

	} // namespace ns_null_pointer_exception
} // namespace util
