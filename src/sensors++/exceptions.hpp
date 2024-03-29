/*
 * exception.hpp
 *
 *  Created on: 20.12.2011
 *      Author: malte
 */

#pragma once
#ifndef SENSORS_EXCEPTIONS_HPP_
#define SENSORS_EXCEPTIONS_HPP_

#include "internal/common.hpp"
#include "util/exception.hpp"
#include <string>
#include <limits>


namespace sensors {

class sensor_error
	: virtual public util::runtime_error
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
		invalid_error = std::numeric_limits<int>::max()
	};

	typedef boost::error_info<struct tag_chip_name, std::string> chip_name;
	typedef boost::error_info<struct tag_feature_name, std::string> feature_name;
	typedef boost::error_info<struct tag_subfeature_name, std::string> subfeature_name;

	static type_enum to_enum(int errnum);

	static const char *strerror(int errnum);

	explicit sensor_error(int errnum);

	explicit sensor_error(const char *message);
	explicit sensor_error(const string_ref &message);
	explicit sensor_error(const std::string &message);

	virtual ~sensor_error();

	const char* what() const noexcept;

	std::string::size_type resource_spec(std::string &s,
			std::string::traits_type::int_type left = std::string::traits_type::eof(),
			std::string::traits_type::int_type right = std::string::traits_type::eof()) const;

	int errnum() const;

	type_enum type() const;

protected:
	int m_errnum;
};


class pwm_error
	: virtual public sensor_error
	, virtual public util::io_error
{
public:
	pwm_error(const char *message);
	pwm_error(const string_ref &message);
	pwm_error(const std::string &message);

	virtual ~pwm_error();

	virtual const char* what() const noexcept;
};



// implementations ========================================

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

} /* namespace sensors */
#endif // SENSORS_EXCEPTIONS_HPP_
