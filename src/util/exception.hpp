/*
 * exceptions.hpp
 *
 *  Created on: 20.12.2011
 *      Author: malte
 */

#pragma once
#ifndef UTIL_EXCEPTIONS_HPP_
#define UTIL_EXCEPTIONS_HPP_

#include <boost/exception/all.hpp>
#include <string>


namespace util {

	class exception_base
		: public virtual std::exception
		, public virtual boost::exception
	{
	public:
		typedef boost::error_info<struct tag_what, std::string> what_t;

		virtual ~exception_base();

		virtual const char *what() const noexcept;

	protected:
		mutable std::string msg;
	};


	struct runtime_error
		: public virtual exception_base
	{
		runtime_error();

		virtual ~runtime_error();

		explicit runtime_error(const std::runtime_error &e);
	};


	namespace ns_null_pointer_exception {

		struct null_pointer_exception
			: public virtual runtime_error
		{
			typedef boost::error_info<struct tag_var_name, const char*> var_name;

			virtual ~null_pointer_exception();
		};


		template <class T>
		typename T::element_type *check(T &p, const char *var_name);

		template <typename T>
		T *check(T *p, const char *var_name);

		extern
		void check(const void *p, const char *var_name);

	} // namespace ns_null_pointer_exception


	typedef ns_null_pointer_exception::null_pointer_exception null_pointer_exception;

#ifndef UTIL_DISABLE_CHECK_POINTER
#	define UTIL_CHECK_POINTER(var) (util::ns_null_pointer_exception::check(var, #var))
#else
#	define UTIL_CHECK_POINTER(var) (var)
#endif


	struct io_error
		: public virtual runtime_error
	{
		typedef boost::error_info<struct tag_errno_code, int> errno_code;
		typedef boost::error_info<struct targ_filename, std::string> filename;

		virtual ~io_error();

		virtual const char *what() const noexcept;
	};



// implementations ========================================

	namespace ns_null_pointer_exception {

		template <class T>
		inline
		typename T::element_type *check(T &p, const char *var_name)
		{
			return check(p.get(), var_name);
		}


		template <typename T>
		inline
		T *check(T *p, const char *var_name)
		{
			check(static_cast<const void*>(p), var_name);
			return p;
		}

	} // namespace ns_null_pointer_exception

} /* namespace util */

#endif /* UTIL_EXCEPTIONS_HPP_ */
