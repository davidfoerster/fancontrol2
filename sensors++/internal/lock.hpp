/*
 * lock.hpp
 *
 *  Created on: 19.12.2011
 *      Author: malte
 */

#pragma once
#ifndef SENSORS_LOCK_HPP_
#define SENSORS_LOCK_HPP_

#include "../csensors.hpp"
#include "../exceptions.hpp"
#include <stdexcept>
#include <memory>
#include <cstdio>
#include <sys/stat.h>


namespace sensors {

	using std::shared_ptr;
	using util::io_error;


	class lock
	{
	public:
		static shared_ptr<lock> instance(bool auto_init = true) throw (sensor_error, io_error);

		~lock();

		bool auto_release() const;
		void auto_release(bool);

		bool initialized() const;
		sensor_error::type_enum init(const char *config = default_config_path) throw (std::logic_error, io_error);

		bool same_config_file(const char *f) const throw (io_error);

		class auto_lock {
		public:
			auto_lock() throw (sensor_error, io_error);

		private:
			shared_ptr<lock> mLock;
		};

	protected:
		void release();

		bool m_initialized, m_auto_release;

	private:
		lock(bool auto_init) throw (sensor_error, io_error);

		sensor_error::type_enum init_internal(const char *config) throw (io_error);

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
