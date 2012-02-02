/*
 * pidfile.cpp
 *
 *  Created on: 31.01.2012
 *      Author: malte
 */

#include "pidfile.hpp"

#include <boost/lexical_cast.hpp>

#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>

namespace meta {


pidfile::pidfile() throw (pidfile_exception, io_error)
	: m_filename("/var/run/fancontrol.pid")
	, m_fd(-1)
{
	int errnum = 0;

	m_fd = ::open(m_filename, O_CREAT|O_EXCL|O_WRONLY, 00644);
	if (m_fd != -1) {
		std::FILE *file = ::fdopen(m_fd, "w");
		if (file) {
			int pid = ::getpid();
			if (std::fprintf(file, "%i\n", pid) > 0 && ::fflush(file) == 0) {
				return;
			} else {
				std::perror("write to PID file");
			}
		} else {
			std::perror("open buffer from PID file descriptor");
		}
	} else {
		std::perror("open PID file");
	}

	errnum = errno;
	close();

	switch (errnum) {
		case EEXIST:
			throw pidfile_exception(*this);

		case EACCES:
			if (::geteuid() != 0) {
				break;
			}
			// fall through

		default:
			throw io_error()
					<< io_error::what_t("Could not access the PID file")
					<< io_error::errno_code(errnum)
					<< io_error::filename(m_filename);
	}
}


pidfile::~pidfile()
{
	close();
}


void pidfile::close()
{
	if (m_fd != -1) {
		if (::unlink(m_filename))
			std::perror("unlink PID file");

		if (::close(m_fd))
			std::perror("close PID file");

		m_fd = -1;
	}
}


pidfile_exception::pidfile_exception(const pidfile &src) throw()
{
	*this << what_t("An instance of fancontrol is running already");

	std::FILE *pidfile = ::fopen(src.m_filename, "r");
	if (pidfile) {
		int pid;
		if (std::fscanf(pidfile, "%i", &pid) == 1) {
			*this << pid_running(pid);
		} else {
			std::perror("parse PID file");
		}

		if (std::fclose(pidfile))
			std::perror("close PID file");
	} else {
		throw io_error() << io_error::errno_code(errno);
	}
}


const char *pidfile_exception::what() const throw()
{
	using boost::exception_detail::get_info;

	if (msg.empty()) {
		exception_base::what();

		const int *pid = boost::exception_detail::get_info<pid_running>::get(*this);
		if (pid) {
			std::string &msg = const_cast<pidfile_exception*>(this)->msg;
			if (!msg.empty()) msg += ' ';
			((msg += "(pid=") += boost::lexical_cast<std::string>(*pid)) += ')';
		}
	}
	return msg.c_str();
}

} /* namespace meta */
