/*
 * lock.cpp
 *
 *  Created on: 19.12.2011
 *      Author: malte
 */

#include "lock.hpp"
#include "../../util/assert.hpp"
#include <cerrno>


static bool operator==(const struct timespec &a, const struct timespec &b)
{
	return a.tv_sec == b.tv_sec && a.tv_nsec == b.tv_nsec;
}


namespace sensors {

	using util::io_error;
	using std::weak_ptr;
	using std::shared_ptr;


	namespace helper {

		static void stat(const char *config, struct stat *statbuf) throw (io_error)
		{
			if (::stat(config, statbuf) != 0)
				BOOST_THROW_EXCEPTION(io_error() << io_error::errno_code(errno));
		}


		static std::FILE *fopen_stat(const char *config, struct stat *statbuf) throw (io_error)
		{
			if (config) {
				std::FILE *const f = std::fopen(config, "r");
				if (f && (!statbuf || ::fstat(::fileno(f), statbuf) == 0))
					return f;
			}
			BOOST_THROW_EXCEPTION(io_error() << io_error::errno_code(errno));
		}


		static bool same_file(const struct stat &a, const struct stat &b)
		{
			return a.st_ino == b.st_ino && a.st_dev == b.st_dev &&
					a.st_ctim == a.st_ctim;
		}

	}


	shared_ptr<lock> lock::instance(bool auto_init) throw (sensor_error, io_error)
	{
		static weak_ptr<lock> &oldlock = *new weak_ptr<lock>();
		if (!oldlock.expired())
			return oldlock.lock();

		shared_ptr<lock> newlock(new lock(auto_init));
		oldlock = newlock;
		return newlock;
	}


	lock::lock(bool auto_init) throw (sensor_error, io_error)
		: m_initialized(false)
		, m_auto_release(true)
		, m_config_file_stat({0})
	{
		if (auto_init) {
			sensor_error::type_enum r = init();
			if (r != sensor_error::no_error)
				BOOST_THROW_EXCEPTION(sensor_error(r));
		}
	}


	lock::~lock()
	{
		if (auto_release()) {
			release();
		}
	}


	sensor_error::type_enum lock::init(const char *config) throw (std::logic_error, io_error)
	{
		if (!m_initialized)
			return init_internal(config);

		if (same_config_file(config))
			return sensor_error::no_error;

		BOOST_THROW_EXCEPTION(std::logic_error("You cannot reinitialise libsensors with a different configuration file whithout releasing it first."));
	}


	sensor_error::type_enum lock::init_internal(const char *config) throw (io_error)
	{
		std::FILE *const f = helper::fopen_stat(config, &m_config_file_stat);
		sensor_error::type_enum r = sensor_error::to_enum(sensors_init(f));
		m_initialized = r == sensor_error::no_error;

		if (!m_initialized)
			sensors_cleanup();

		return r;
	}


	bool lock::same_config_file(const char *f) const throw (io_error)
	{
		struct stat statbuf;
		helper::stat(f, &statbuf);
		return m_initialized && helper::same_file(statbuf, m_config_file_stat);
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
	{ }

} /* namespace sensors */
