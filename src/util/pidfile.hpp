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
#include <unistd.h>


namespace util {

	class pidfile;


	struct pidfile_exception
		: virtual public runtime_error
	{
		typedef boost::error_info<struct tag_pid_running, ::pid_t> pid_running;

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

		static ::pid_t read(std::FILE* f);
	private:
		pidfile(const pidfile&) = delete;

		pidfile &operator=(const pidfile&) = delete;

		int open_exclusively(const char *&perror_msg);

		bool delete_stale();

		::pid_t read();

		bool close();

		bool unlink(bool always = false);

		bool cleanup();

		std::FILE *m_file;

		stringpiece_alloc m_filename;

		friend class pidfile_exception;
	};

} /* namespace util */
#endif /* UTIL_PIDFILE_HPP_ */
