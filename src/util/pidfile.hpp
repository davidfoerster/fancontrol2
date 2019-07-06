/*
 * pidfile.hpp
 *
 *  Created on: 31.01.2012
 *      Author: malte
 */

#pragma once
#ifndef UTIL_PIDFILE_HPP_
#define UTIL_PIDFILE_HPP_

#include "stdio_fstream.hpp"
#include "stringpiece/stringpiece.hpp"
#include "exception.hpp"
#include <unistd.h>


namespace util {

	class pidfile;


	struct pidfile_exception
		: virtual public runtime_error
	{
		typedef boost::error_info<struct tag_pid_running, ::pid_t> pid_running;

		explicit pidfile_exception(pidfile &src);

		virtual const char *what() const noexcept;
	};


	class pidfile
	{
	public:
		explicit pidfile(const stringpiece &filename, bool root_only = false);

		~pidfile();

	private:
		pidfile(const pidfile&) = delete;

		pidfile &operator=(const pidfile&) = delete;

		void open_exclusively();

		bool delete_stale();

		::pid_t read();

		static ::pid_t read(std::basic_istream<char> &f);

		bool unlink(bool always = false);

		void cleanup();

		stdio_fstream m_file;

		stringpiece_alloc m_filename;

		friend class pidfile_exception;
	};

} /* namespace util */

#endif /* UTIL_PIDFILE_HPP_ */
