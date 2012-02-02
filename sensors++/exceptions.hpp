/*
 * exception.hpp
 *
 *  Created on: 20.12.2011
 *      Author: malte
 */

#pragma once
#ifndef SENSORS_EXCEPTION_HPP_
#define SENSORS_EXCEPTION_HPP_

#include "internal/common.hpp"
#include "meta/exception.hpp"
#include <boost/integer_traits.hpp>
#include <string>

namespace sensors {


class sensor_error
	: virtual public meta::runtime_error
{
public:
	enum type_enum {
		no_error = 0,
		misplaced_wildcard = 1,
		subfeature_unknown = 2,
		no_read_access = 3,
		kernel_interface_error = 4,
		divide_by_zero = 5,
		unparsable_chip_name = 6,
		unparsable_bus_name = 7,
		unparsable_general = 8,
		no_write_access = 9,
		io_error = 10,
		recursion_too_deep = 11,
		_end,
		invalid_error = boost::integer_traits<int>::const_max
	};

	typedef boost::error_info<struct tag_chip_name, std::string> chip_name;
	typedef boost::error_info<struct tag_feature_name, std::string> feature_name;
	typedef boost::error_info<struct tag_subfeature_name, std::string> subfeature_name;

	static type_enum to_enum(int errnum);

	static const char *strerror(int errnum);

	explicit sensor_error(int errnum);

	explicit sensor_error(const char *message) throw();
	explicit sensor_error(const string_ref &message) throw();
	explicit sensor_error(const std::string &message) throw();

	//virtual ~sensor_error() throw();

	const char* what() const throw();

	int errnum() const;

	type_enum type() const;

protected:
	int m_errnum;
};


class pwm_error
	: virtual public sensor_error, virtual public meta::io_error
{
public:
	pwm_error(const char *message) throw();
	pwm_error(const string_ref &message) throw();
	pwm_error(const std::string &message) throw();

	//virtual ~pwm_error() throw();

	virtual const char* what() const throw() { return sensor_error::what(); }
};



// implementations ========================================

/*inline
sensor_error::~sensor_error() throw()
{
}*/


inline
int sensor_error::errnum() const
{
	return m_errnum;
}


inline
sensor_error::type_enum sensor_error::type() const
{
	return to_enum(m_errnum);
}


/*inline
pwm_error::~pwm_error() throw()
{
}*/

} /* namespace sensors */
#endif /* SENSORS_EXCEPTION_HPP_ */
