/*
 * pidfile.hpp
 *
 *  Created on: 31.01.2012
 *      Author: malte
 */

#pragma once
#ifndef PIDFILE_HPP_
#define PIDFILE_HPP_

#include "exception.hpp"

namespace meta {

class pidfile;


struct pidfile_exception
	: virtual public runtime_error
{
	typedef boost::error_info<struct tag_pid_running, int> pid_running;

	pidfile_exception(const pidfile &src) throw ();

	virtual const char *what() const throw();
};


class pidfile
{
public:
	pidfile() throw (pidfile_exception, io_error);

	~pidfile();

private:
	void close();

	const char *const m_filename;

	int m_fd;

	friend class pidfile_exception;
};

} /* namespace meta */
#endif /* PIDFILE_HPP_ */
