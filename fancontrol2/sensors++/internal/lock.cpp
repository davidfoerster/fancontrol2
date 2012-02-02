/*
 * lock.cpp
 *
 *  Created on: 19.12.2011
 *      Author: malte
 */

#include "lock.hpp"

#include <boost/weak_ptr.hpp>
#include <boost/make_shared.hpp>

#include "../csensors.hpp"
#include <cerrno>

namespace sensors {

using boost::weak_ptr;
using boost::shared_ptr;


shared_ptr<lock> lock::instance(bool auto_init) throw (sensor_error, io_error)
{
	static weak_ptr<lock> &oldlock = *new weak_ptr<lock>();
	if (!oldlock.expired())
		return oldlock.lock();

	shared_ptr<lock> newlock(boost::make_shared<lock>(auto_init));
	oldlock = newlock;
	return newlock;
}


lock::lock(bool auto_init) throw (sensor_error, io_error)
	: m_initialized(false)
	, m_auto_release(true)
{
	if (auto_init) {
		sensor_error::type_enum r = init();
		if (r != sensor_error::no_error)
			throw sensor_error(r);
	}
}


lock::~lock()
{
	if (auto_release()) {
		release();
	}
}


sensor_error::type_enum lock::init(std::FILE *config) throw (std::logic_error, io_error)
{
	if (!m_initialized)
		return init_internal(config);

	if (same_config_file(config))
		return sensor_error::no_error;

	throw std::logic_error("You cannot reinitialise libsensors with a different configuration file whithout releasing it first.");
}


sensor_error::type_enum lock::init_internal(std::FILE *config) throw (io_error)
{
	configfile_stat(config, m_config_file_stat);
	sensor_error::type_enum r = sensor_error::to_enum(sensors_init(config));
	m_initialized = r == sensor_error::no_error;
	return r;
}


void lock::configfile_stat(std::FILE *config, struct stat &buf) throw (io_error)
{
	if ((config ? ::fstat(::fileno(config), &buf) : ::stat("/etc/sensors3.conf", &buf)) != 0) {
		BOOST_THROW_EXCEPTION(io_error() << io_error::errno_code(errno));
	}
}


bool operator==(const struct timespec &a, const struct timespec &b)
{
	return a.tv_sec == b.tv_sec && a.tv_nsec == b.tv_nsec;
}


bool same_file(const struct stat &a, const struct stat &b)
{
	return a.st_ino == b.st_ino && a.st_dev == b.st_dev &&
			a.st_ctim == b.st_ctim;
}


bool lock::same_config_file(FILE *f) const throw (io_error)
{
	struct stat statbuf;
	configfile_stat(f, statbuf);
	return m_initialized && same_file(statbuf, m_config_file_stat);
}


void lock::release()
{
	if (initialized()) {
		m_initialized = false;
		sensors_cleanup();
	}
}


lock::auto_lock::auto_lock() throw (sensor_error, io_error)
	: mLock(lock::instance(true))
{
}


lock::auto_lock &lock::auto_lock::operator=(const auto_lock &o)
{
	shared_ptr<lock>(o.mLock).swap(this->mLock);
	return *this;
}

} /* namespace sensors */
