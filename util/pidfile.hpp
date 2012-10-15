/*
 * pidfile.hpp
 *
 *  Created on: 31.01.2012
 *      Author: malte
 */

#pragma once
#ifndef UTIL_PIDFILE_HPP_
#define UTIL_PIDFILE_HPP_

#include "stringpiece/stringpiece.hpp"
#include "exception.hpp"
#include <cstdio>


namespace util {

	class pidfile;


	struct pidfile_exception
	    : virtual public runtime_error
	{
	    typedef ::boost::error_info<struct tag_pid_running, int> pid_running;

	    explicit pidfile_exception(pidfile &src) throw ();

	    virtual const char *what() const throw();
	};


	class pidfile
	{
	public:
	    explicit pidfile(const stringpiece &filename, bool root_only = false)
		    throw (pidfile_exception, io_error);

	    pidfile(pidfile&);

	    pidfile &operator=(pidfile&);

	    ~pidfile();

	private:
	    pidfile(const pidfile&) throw();

	    pidfile &operator=(const pidfile&) throw();

	    void close();

	    void unlink();

	    void cleanup();

	    ::std::FILE *m_file;

	    stringpiece_alloc m_filename;

	    friend class pidfile_exception;
	};

} /* namespace util */
#endif /* UTIL_PIDFILE_HPP_ */
