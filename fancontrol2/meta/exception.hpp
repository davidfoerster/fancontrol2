/*
 * exceptions.hpp
 *
 *  Created on: 20.12.2011
 *      Author: malte
 */

#pragma once
#ifndef META_EXCEPTIONS_HPP_
#define META_EXCEPTIONS_HPP_

#include <boost/exception/all.hpp>
#include <string>

namespace meta {


class exception_base
	: public virtual std::exception
	, public virtual boost::exception
{
public:
	typedef boost::error_info<struct tag_what, std::string> what_t;

	virtual ~exception_base() throw() {}

	virtual const char *what() const throw();

protected:
	std::string msg;
};

struct runtime_error
	: public virtual exception_base
{
	runtime_error() throw() {}

	explicit runtime_error(const std::runtime_error &e);
};

struct null_pointer_exception
	: public virtual runtime_error
{
	typedef boost::error_info<struct tag_var_name, const char*> var_name;

	template <class T>
	static typename T::element_type *check(T &p, const char *var_name) throw (null_pointer_exception);

	template <typename T>
	static T *check(T *p, const char *var_name) throw (null_pointer_exception);

#	define META_CHECK_POINTER(var) (::meta::null_pointer_exception::check(var, #var))

};

struct io_error
	: public virtual meta::runtime_error
{
	typedef boost::error_info<struct tag_errno_code, int> errno_code;
	typedef boost::error_info<struct targ_filename, std::string> filename;

	virtual const char *what() const throw();
};



// implementations ========================================

template <class T> inline
typename T::element_type *null_pointer_exception::check(T &p, const char *var_name)
throw (null_pointer_exception)
{
	return check(p.get(), var_name);
}


template <typename T> inline
T *null_pointer_exception::check(T *p, const char *var_name)
throw (null_pointer_exception)
{
	if (!p) {
		BOOST_THROW_EXCEPTION(null_pointer_exception() << null_pointer_exception::var_name(var_name));
	}
	return p;
}

} /* namespace meta */
#endif /* META_EXCEPTIONS_HPP_ */
