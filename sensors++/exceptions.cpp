/*
 * SensorError.cpp
 *
 *  Created on: 20.12.2011
 *      Author: malte
 */

#include "exceptions.hpp"
#include "../util/algorithm.hpp"
#include "../util/in_range.hpp"
#include "../util/assert.hpp"
#include <cstring>

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
	    *this << what_t(message.str());
	}


	sensor_error::sensor_error(const ::std::string &message) throw()
	    : m_errnum(invalid_error)
	{
	    *this << what_t(message);
	}


	sensor_error::~sensor_error() throw()
	{ }


	const char* sensor_error::what() const throw()
	{
	    if (msg.empty()) {
		    exception_base::what();
		    ::std::string &msg = const_cast<sensor_error*>(this)->msg;

		    type_enum type = to_enum(m_errnum);
		    if (type != invalid_error) {
			    if (!msg.empty()) (msg += ':') += ' ';
			    msg += sensors_strerror(type);
		    }

		    resource_spec(msg, '(', ')');
	    }
	    return msg.c_str();
	}


	::std::string::size_type sensor_error::resource_spec(::std::string &s,
		    ::std::string::traits_type::int_type left,
		    ::std::string::traits_type::int_type right)
	    const
	{
	    using ::boost::exception_detail::get_info;
	    ::std::string::size_type old_length = s.length();

	    const ::std::string *chip_name = get_info<sensor_error::chip_name>::get(*this);
	    if (chip_name) {
		    if (!s.empty())
			    s += ' ';
		    if (left != ::std::string::traits_type::eof())
			    s += ::std::string::traits_type::to_char_type(left);

		    (s += "on ") += *chip_name;

		    const ::std::string *feature_name = get_info<sensor_error::feature_name>::get(*this);
		    if (feature_name) {
			    (s += '/') += *feature_name;

			    const ::std::string *subfeature_name = get_info<sensor_error::subfeature_name>::get(*this);
			    if (subfeature_name) {
				    (s += '/') += *subfeature_name;
			    }
		    }

		    if (right != ::std::string::traits_type::eof())
			    s += ::std::string::traits_type::to_char_type(right);
	    }

	    return s.length() - old_length;
	}


	sensor_error::type_enum sensor_error::to_enum(int errnum)
	{
	    if (errnum < 0) errnum = -errnum;
	    return ::util::in_range<int>(errnum, 0, _end) ? static_cast<type_enum>(errnum) : invalid_error;
	}


	pwm_error::pwm_error(const char *message) throw()
	    : sensor_error(message)
	{
	}


	pwm_error::pwm_error(const string_ref &message) throw()
	    : sensor_error(message)
	{
	}


	pwm_error::pwm_error(const ::std::string &message) throw()
	    : sensor_error(message)
	{
	}


	pwm_error::~pwm_error() throw()
	{ }


	const char* pwm_error::what() const throw()
	{
	    if (msg.empty()) {
		    io_error::what();
		    resource_spec(const_cast<pwm_error*>(this)->msg, '(', ')');
	    }
	    return msg.c_str();
	}

} /* namespace sensors */
