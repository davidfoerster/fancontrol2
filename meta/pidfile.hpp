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
#include <chromium/base/string_piece.h>
#include <string>

#include <cstdio>

namespace meta {

class pidfile;


struct pidfile_exception
	: virtual public runtime_error
{
	typedef boost::error_info<struct tag_pid_running, int> pid_running;

	explicit pidfile_exception(pidfile &src) throw ();

	virtual const char *what() const throw();
};


class pidfile
{
public:
	typedef chromium::base::StringPiece string_ref;

	explicit pidfile(const string_ref &filename, bool root_only = false, bool copy_filename = true)
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

	static string_ref strcpy_alloc(const string_ref &s);

	std::FILE *m_file;

	bool m_delete_filename;

	string_ref m_filename;

	friend class pidfile_exception;
};

} /* namespace meta */
#endif /* PIDFILE_HPP_ */
