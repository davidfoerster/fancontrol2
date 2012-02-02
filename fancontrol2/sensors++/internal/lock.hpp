/*
 * lock.hpp
 *
 *  Created on: 19.12.2011
 *      Author: malte
 */

#pragma once
#ifndef SENSORS_LOCK_HPP_
#define SENSORS_LOCK_HPP_

#include "../exceptions.hpp"
#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include <cstdio>
#include <sys/stat.h>

namespace boost {

template <typename> class shared_ptr;

template <class T, typename A> shared_ptr<T> make_shared(A const &);

}

namespace sensors {

using boost::shared_ptr;
using meta::io_error;


class lock
{
public:
	static shared_ptr<lock> instance(bool auto_init = true) throw (sensor_error, io_error);

	~lock();

	bool auto_release() const;
	void auto_release(bool);

	bool initialized() const;
	sensor_error::type_enum init(std::FILE *config = 0) throw (std::logic_error, io_error);

	bool same_config_file(std::FILE *f) const throw (io_error);

	class auto_lock {
	public:
		auto_lock() throw (sensor_error, io_error);

		auto_lock &operator=(const auto_lock &o);

	private:
		shared_ptr<lock> mLock;
	};

protected:
	void release();

	bool m_initialized, m_auto_release;

private:
	lock(bool auto_init) throw (sensor_error, io_error);

	friend shared_ptr<lock> boost::make_shared<lock, bool>(bool const &);

	sensor_error::type_enum init_internal(std::FILE *config = 0) throw (io_error);

	static void configfile_stat(std::FILE *config, struct stat &buf) throw (io_error);

	struct stat m_config_file_stat;
};



// implementations ========================================

inline
bool lock::auto_release() const
{
	return m_auto_release;
}


inline
bool lock::initialized() const
{
	return m_initialized;
}



} /* namespace sensors */
#endif /* SENSORS_LOCK_HPP_ */
