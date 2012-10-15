/*
 * pidfile.cpp
 *
 *  Created on: 31.01.2012
 *      Author: malte
 */

#include "pidfile.hpp"
#include "logging.hpp"
#include "strcat.hpp"

#include <boost/lexical_cast.hpp>
#include <algorithm>
#include <stdexcept>

#include <fcntl.h>
#include <unistd.h>
#include <cerrno>

namespace util {

	pidfile::pidfile(const stringpiece &filename, bool root_only)
		    throw (pidfile_exception, io_error)
	    : m_filename(filename)
	{
	    using namespace std;

	    BOOST_ASSERT(!filename.empty());

	    int errnum = 0,
		    &perrno = errno;

	    if (!root_only || ::geteuid() == 0) {
		    // try to open exclusively
		    m_file = fopen(m_filename.c_str(), "wx");
		    if (m_file) {
			    if (fprintf(m_file, "%i\n", ::getpid()) > 1) {
				    if (fflush(m_file) == 0) {
					    return;
				    } else {
					    UTIL_DEBUG(perror("flush PID file"));
				    }
			    } else {
				    UTIL_DEBUG(perror("write to PID file"));
			    }
		    } else {
			    UTIL_DEBUG(perror("exclusively open PID file for writing"));
		    }

	    } else {
		    // don't try to write, if we are not allowed to anyway
		    // instead, try to open fo reading
		    perrno = 0;
		    m_file = fopen(m_filename.c_str(), "r");
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
				    << io_error::filename(m_filename.str()));
	    }
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


	void pidfile::unlink()
	{
	    if (m_file) {
		    if (::unlink(m_filename.c_str()) != 0)
			    UTIL_DEBUG(::std::perror("unlink PID file"));
	    }
	}


	void pidfile::close()
	{
	    if (m_file) {
		    if (::std::fclose(m_file) != 0)
			    UTIL_DEBUG(::std::perror("close PID file"));
		    m_file = nullptr;
	    }
	}


	void pidfile::cleanup()
	{
	    unlink();
	    close();
	}


	pidfile::pidfile(const pidfile &o) throw()
	{
	    BOOST_THROW_EXCEPTION(::std::logic_error("Invalid operation"));
	}


	pidfile &pidfile::operator=(const pidfile &o) throw()
	{
	    BOOST_THROW_EXCEPTION(::std::logic_error("Invalid operation"));
	}


	pidfile_exception::pidfile_exception(pidfile &src) throw()
	{
	    using namespace std;

	    *this << what_t("An instance of fancontrol is running already");

	    if (!src.m_file)
		    src.m_file = fopen(src.m_filename.c_str(), "r");

	    if (src.m_file) {
		    int pid;
		    if (fscanf(src.m_file, "%i", &pid) == 1) {
			    *this << pid_running(pid);
		    } else {
			    UTIL_DEBUG(perror("parse PID file"));
		    }
		    // we rely on it being closed by pidfile
	    } else {
		    BOOST_THROW_EXCEPTION(io_error() << io_error::errno_code(errno));
	    }
	}


	const char *pidfile_exception::what() const throw()
	{
	    using ::boost::exception_detail::get_info;

	    if (msg.empty()) {
		    exception_base::what();

		    const int *const pid = ::boost::exception_detail::get_info<pid_running>::get(*this);
		    if (pid) {
			    if (!msg.empty()) msg += ' ';
			    ((msg += "(pid=") << *pid) += ')';
		    }
	    }
	    return msg.c_str();
	}

} /* namespace util */
