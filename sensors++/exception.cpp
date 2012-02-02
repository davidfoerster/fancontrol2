/*
 * SensorError.cpp
 *
 *  Created on: 20.12.2011
 *      Author: malte
 */

#include "exceptions.hpp"
#include "meta/algorithm.hpp"

#include "meta/assert.hpp"

namespace sensors {
#include <sensors/error.h>


const char *sensor_error::strerror(int errnum)
{
	if (errnum < 0) {
		errnum = -errnum;
		if (errnum < 0) errnum -= 1;
	}
	return BOOST_VERIFY_R(errnum < sensor_error::_end) ? sensors_strerror(errnum) : "Invalid error";
}


sensor_error::sensor_error(int errnum)
	: m_errnum(errnum)
{
	*this << what_t(strerror(errnum));
}


sensor_error::sensor_error(const char *message) throw()
	: m_errnum(invalid_error)
{
	*this << what_t(message);
}


sensor_error::sensor_error(const string_ref &message) throw()
	: m_errnum(invalid_error)
{
	*this << what_t(message.as_string());
}


sensor_error::sensor_error(const std::string &message) throw()
	: m_errnum(invalid_error)
{
	*this << what_t(message);
}


const char* sensor_error::what() const throw()
{
	using boost::exception_detail::get_info;

	if (msg.empty()) {
		exception_base::what();
		std::string &msg = const_cast<sensor_error*>(this)->msg;

		type_enum type = to_enum(m_errnum);
		if (type != invalid_error) {
			if (!msg.empty()) (msg += ':') += ' ';
			msg += sensors_strerror(type);
		}

		const std::string *chip_name = get_info<sensor_error::chip_name>::get(*this);
		if (chip_name) {
			if (!msg.empty()) msg += ' ';
			(msg += "(on ") += *chip_name;

			const std::string *feature_name = get_info<sensor_error::feature_name>::get(*this);
			if (feature_name) {
				(msg += '/') += *feature_name;

				const std::string *subfeature_name = get_info<sensor_error::subfeature_name>::get(*this);
				if (subfeature_name) {
					(msg += '/') += *subfeature_name;
				}
			}
			msg += ')';
		}
	}
	return msg.c_str();
}


sensor_error::type_enum sensor_error::to_enum(int errnum)
{
	using meta::in_range;

	if (errnum < 0) errnum = -errnum;
	if (BOOST_VERIFY_R(in_range<int>(errnum, 0, _end))) {
		errnum = invalid_error;
	}
	return static_cast<type_enum>(errnum);
}


pwm_error::pwm_error(const char *message) throw()
	: sensor_error(message)
{
}


pwm_error::pwm_error(const string_ref &message) throw()
	: sensor_error(message)
{
}


pwm_error::pwm_error(const std::string &message) throw()
	: sensor_error(message)
{
}

} /* namespace sensors */
