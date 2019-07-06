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
#include <boost/assert.hpp>
#include <cerrno>
#include <unistd.h>
#include <signal.h>


namespace util {

	pidfile::pidfile(const stringpiece &filename, bool root_only)
		: m_file()
		, m_filename(filename)
	{
		using std::ios;

		int &perrno = errno;
		perrno = 0;
		BOOST_ASSERT(!filename.empty());

		if (!root_only || ::geteuid() == 0) {
			// try to open exclusively
			open_exclusively();
			if ((m_file.rdstate() & ios::failbit) && perrno == EEXIST) {
				// delete PID file and try again
				m_file.clear();
				if (delete_stale()) {
					open_exclusively();
				} else {
					perrno = EEXIST;
				}
			}

			if (m_file.is_open() && m_file.good()) {
				return;
			} else if (!m_file && perrno != EEXIST) {
				UTIL_DEBUG(std::perror(nullptr));
			}

		} else {
			// don't try to write, if we are not allowed to anyway
			// instead, try to open for reading
			m_file.open(m_filename.c_str(), ios::in, 0, 0, 16);
			if (m_file.good()) {
				perrno = EEXIST;
			} else if (perrno == ENOENT) {
				// the file does not exist; this is good
				perrno = 0;
				return;
			}
			// don't unlink and close yet, we may still want to read the content in pidfile_exception()
		}

		std::unique_ptr<exception_base> ex;
		if (perrno == EEXIST) {
			ex.reset(new pidfile_exception(*this));
		} else {
			ex.reset(new io_error());
			*ex << io_error::what_t("Could not access the PID file")
					<< io_error::errno_code(perrno)
					<< io_error::filename(m_filename.str());
		}
		cleanup();
		BOOST_THROW_EXCEPTION(*ex);
	}


	void pidfile::open_exclusively()
	{
		using std::ios;
		m_file.open(m_filename.c_str(), std::ios::out, O_CREAT|O_EXCL,
				S_IRUSR|S_IRGRP|S_IROTH, 16);
		if (m_file.good()) {
			m_file.exceptions(ios::failbit | ios::badbit | ios::eofbit);
			( m_file << ::getpid() << std::endl ).flush();
		}
	}


	::pid_t pidfile::read()
	{
		std::basic_istream<char> *const f =
				m_file.is_open() ?
					static_cast<std::basic_istream<char>*>(&m_file) :
					new std::ifstream(m_filename.c_str(), std::ios::in);
		::pid_t pid = pidfile::read(*f);
		if (f != &m_file)
			delete f;
		return pid;
	}


	::pid_t pidfile::read(std::basic_istream<char> &f)
	{
		::pid_t pid = -1;
		if (!!f) {
			f.seekg(0);
			f >> pid;
			if (!f || pid < 0)
				pid = 0;
		}
		return pid;
	}


	bool pidfile::delete_stale()
	{
		::pid_t pid = this->read();
		if (pid < 0)
			return true;
		if (pid == 0 || (::kill(pid, 0) != 0 && errno == ESRCH)) {
			m_file.close();
			return this->unlink(true);
		}
		return false;
	}


	pidfile::~pidfile()
	{
		this->unlink(false);
	}


	bool pidfile::unlink(bool always)
	{
		if (always || (m_file.mode() & std::ios::out)) {
			if (::unlink(m_filename.c_str()) != 0) {
				UTIL_DEBUG(std::perror("unlink PID file"));
				return false;
			}
		}
		return true;
	}


	void pidfile::cleanup()
	{
		unlink(false);
		m_file.close();
	}


	pidfile_exception::pidfile_exception(pidfile &src)
	{
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


	const char *pidfile_exception::what() const noexcept
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
