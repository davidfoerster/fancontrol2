/*
 * pidfile.cpp
 *
 *  Created on: 31.01.2012
 *      Author: malte
 */

#include "pidfile.hpp"

#include "meta/utility.hpp"
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include <stdexcept>

#include <fcntl.h>
#include <unistd.h>
#include <cerrno>

namespace meta {


pidfile::pidfile(const string_ref &filename, bool root_only, bool copy_filename)
		throw (pidfile_exception, io_error)
	: m_delete_filename(copy_filename || *filename.end() != 0)
	, m_filename(m_delete_filename ? strcpy_alloc(filename) : filename)
{
	BOOST_ASSERT(!filename.empty());

	int errnum = 0,
		&perrno = errno;

	if (!root_only || ::geteuid() == 0) {
		// try to open exclusively
		m_file = std::fopen(m_filename.data(), "wx");
		if (m_file) {
			if (std::fprintf(m_file, "%i", ::getpid()) > 0 && std::fputc('\n', m_file) != EOF) {
				if (std::fflush(m_file) == 0) {
					return;
				} else {
					META_DEBUG(std::perror("flush PID file"));
				}
			} else {
				META_DEBUG(std::perror("write to PID file"));
			}
		} else {
			META_DEBUG(std::perror("exclusively open PID file for writing"));
		}

	} else {
		// don't try to write, if we are not allowed to anyway
		// instead, try to open fo reading
		perrno = 0;
		m_file = std::fopen(m_filename.data(), "r");
		if (m_file) {
			perrno = EEXIST;
		} else if (perrno == ENOENT) {
			// the file does not exist; this is good
			perrno = 0;
			return;
		}
		// don't unlink and close yet, we may still want to read the content in pidfile_exception()
	}

	errnum = perrno;
	if (errnum == EEXIST) {
		pidfile_exception ex(*this);
		cleanup();
		BOOST_THROW_EXCEPTION(ex);
	} else {
		cleanup();
		BOOST_THROW_EXCEPTION(io_error()
				<< io_error::what_t("Could not access the PID file")
				<< io_error::errno_code(errnum)
				<< io_error::filename(m_filename.as_string()));
	}
}


pidfile::pidfile(pidfile &o)
	: m_file(o.m_file)
	, m_delete_filename(o.m_delete_filename)
	, m_filename(o.m_filename)
{
	o.m_file = 0;
	if (o.m_delete_filename) {
		o.m_delete_filename = false;
		o.m_filename.clear();
	}
}


pidfile &pidfile::operator=(pidfile &o)
{
	m_file = o.m_file; o.m_file = 0;
	m_delete_filename = o.m_delete_filename;
	m_filename = o.m_filename;
	if (o.m_delete_filename) {
		o.m_delete_filename = false;
		o.m_filename.clear();
	}

	return *this;
}


pidfile::string_ref pidfile::strcpy_alloc(const string_ref &s)
{
	char *buf = new char[s.length() + 1];
	s.copy(buf, s.length());
	buf[s.length()] = 0;
	return string_ref(buf, s.length());
}


pidfile::~pidfile()
{
	cleanup();
}


void pidfile::unlink()
{
	if (m_file) {
		if (::unlink(m_filename.data()) != 0)
			META_DEBUG(std::perror("unlink PID file"));
	}
}


void pidfile::close()
{
	if (m_file) {
		if (std::fclose(m_file) != 0)
			META_DEBUG(std::perror("close PID file"));
		m_file = 0;
	}
}


void pidfile::cleanup()
{
	unlink();
	close();

	if (m_delete_filename) {
		delete[] const_cast<char*>(m_filename.data());
#ifndef NDEBUG
		m_filename.clear();
		m_delete_filename = false;
#endif
	}
}


pidfile::pidfile(const pidfile &o) throw()
{
	BOOST_THROW_EXCEPTION(std::logic_error("Invalid operation"));
}


pidfile &pidfile::operator=(const pidfile &o) throw()
{
	BOOST_THROW_EXCEPTION(std::logic_error("Invalid operation"));
}


pidfile_exception::pidfile_exception(pidfile &src) throw()
{
	*this << what_t("An instance of fancontrol is running already");

	if (!src.m_file) src.m_file = std::fopen(src.m_filename.data(), "r");
	if (src.m_file) {
		int pid;
		if (std::fscanf(src.m_file, "%i", &pid) == 1) {
			*this << pid_running(pid);
		} else {
			META_DEBUG(std::perror("parse PID file"));
		}
		// we rely on it being closed by pidfile
	} else {
		BOOST_THROW_EXCEPTION(io_error() << io_error::errno_code(errno));
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
