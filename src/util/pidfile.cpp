/*
 * pidfile.cpp
 *
 *  Created on: 31.01.2012
 *      Author: malte
 */

#include "pidfile.hpp"
#include "logging.hpp"
#include "strcat.hpp"
#include "preprocessor.hpp"

#include <limits>
#include <cerrno>
#include <cctype>

#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

namespace util {

	pidfile::pidfile(const stringpiece &filename, bool root_only)
			throw (pidfile_exception, io_error)
		: m_file(nullptr)
		, m_filename(filename)
	{
		using namespace std;

		int &perrno = errno;
		BOOST_ASSERT(!filename.empty());


		if (!root_only || ::geteuid() == 0) {
			// try to open exclusively
			const char *perror_msg = nullptr;
			int r;
			if ((r = open_exclusively(perror_msg)) == 1) {
				// delete PID file and try again
				r = delete_stale() ? open_exclusively(perror_msg) : -1;
			}
			switch (r) {
			case 0:
						return;
			case 1:
				perrno = EEXIST;
				break;
			default:
				UTIL_DEBUG(std::perror(perror_msg));
				break;
			}

		} else {
			// don't try to write, if we are not allowed to anyway
			// instead, try to open fo reading
			perrno = 0;
			m_file = std::fopen(m_filename.c_str(), "r");
			if (m_file) {
				perrno = EEXIST;
			} else if (perrno == ENOENT) {
				// the file does not exist; this is good
				perrno = 0;
				return;
			}
			// don't unlink and close yet, we may still want to read the content in pidfile_exception()
		}

		int errnum = perrno;
		if (errnum == EEXIST) {
			pidfile_exception ex(*this);
			cleanup();
			BOOST_THROW_EXCEPTION(ex);
		} else {
			cleanup();
			BOOST_THROW_EXCEPTION(io_error()
					<< io_error::what_t("Could not access the PID file")
					<< io_error::errno_code(errnum)
					<< io_error::filename(m_filename.str()));
		}
	}


	int pidfile::open_exclusively(const char *&errmsg)
	{
		int fd;
		if ((fd = ::open(m_filename.c_str(), O_WRONLY|O_CREAT|O_EXCL, S_IRUSR|S_IRGRP|S_IROTH)) < 0) {
			errmsg = "exclusively open PID file for writing";
			if (errno == EEXIST)
				return 1;
		} else if (!(m_file = ::fdopen(fd, "w"))) {
			errmsg = "initialize PID file buffer structure";
			::close(fd);
		} else if (std::fprintf(m_file, "%i\n", ::getpid()) < 2) {
			errmsg = "write to PID file";
		} else if (std::fflush(m_file) != 0) {
			errmsg = "flush PID file";
		} else {
			return 0;
		}

		return -1;
	}


	::pid_t pidfile::read()
	{
		std::FILE *const f = m_file ? m_file : std::fopen(m_filename.c_str(), "r");
		::pid_t pid = pidfile::read(f);
		if (f != m_file) std::fclose(f);
		return pid;
	}


	::pid_t pidfile::read(std::FILE *f)
	{
		if (!f)
			return -1;
		//std::rewind(f);

		unsigned long pid;
		if (std::fscanf(f, "%lu", &pid) == 1) {
			if (pid > 0 && pid <= std::numeric_limits<::pid_t>::max()) {
				return static_cast<::pid_t>(pid);
			}
		}

		return 0;
	}


	bool pidfile::delete_stale()
	{
		::pid_t pid = this->read();
		if (pid < 0)
			return true;
		if (pid == 0 || (::kill(pid, 0) != 0 && errno == ESRCH)) {
			this->close();
			return this->unlink(true);
		}
		return false;
	}
	pidfile::pidfile(pidfile &o)
	{
		operator=(o);
	}


	pidfile &pidfile::operator=(pidfile &o)
	{
		cleanup();
		m_file = o.m_file;
		o.m_file = nullptr;
		m_filename = o.m_filename;
		return *this;
	}


	pidfile::~pidfile()
	{
		cleanup();
	}


	bool pidfile::unlink(bool always)
	{
		if (always || m_file) {
			if (::unlink(m_filename.c_str()) != 0) {
				UTIL_DEBUG(std::perror("unlink PID file"));
				return false;
		}
	}
		return true;
	}


	bool pidfile::close()
	{
		if (m_file) {
			bool success = std::fclose(m_file) == 0;
			if (!success)
				UTIL_DEBUG(std::perror("close PID file"));
			m_file = nullptr;
			return success;
		}
		return true;
	}


	bool pidfile::cleanup()
	{
		bool b;
		b  = unlink();
		b &= close();
		return b;
	}


	pidfile_exception::pidfile_exception(pidfile &src) throw()
	{
		using namespace std;

		*this << what_t("An instance of fancontrol is running already");

		const ::pid_t pid = src.read();
		if (pid > 0) {
				*this << pid_running(pid);
			} else {
			const int errnum = errno;
			if (pid == 0)
				UTIL_DEBUG(perror("parse PID file"));
			BOOST_THROW_EXCEPTION(io_error() << io_error::errno_code(errnum));
		}
	}


	const char *pidfile_exception::what() const throw()
	{
		using boost::exception_detail::get_info;

		if (msg.empty()) {
			exception_base::what();

			const int *const pid = boost::exception_detail::get_info<pid_running>::get(*this);
			if (pid) {
				if (!msg.empty()) msg += ' ';
				((msg += "(pid=") << *pid) += ')';
			}
		}
		return msg.c_str();
	}

} /* namespace util */
