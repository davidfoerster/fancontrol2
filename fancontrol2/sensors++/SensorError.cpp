/*
 * SensorError.cpp
 *
 *  Created on: 20.12.2011
 *      Author: malte
 */

#include "SensorError.hpp"
#include <cassert>

namespace sensors {
#include <sensors/error.h>


const char *SensorError_strerror(int errnum)
{
	if (errnum < 0) errnum = -errnum;
	assert(errnum < SensorError::_end);
	return (errnum < SensorError::_end) ? sensors_strerror(errnum) : "Invalid error";
}


SensorError::SensorError(int errnum) throw()
	: errorMessage(SensorError_strerror(errnum))
	, errnum(errnum)
{
}


SensorError::SensorError(const char *message) throw()
	: errorMessage(message)
	, errnum(invalid_error)
{
}


SensorError::EType SensorError::toEnum(int errnum)
{
	if (errnum < 0) errnum = -errnum;
	if (errnum >= _end) {
		assert(errnum < _end);
		errnum = invalid_error;
	}
	return static_cast<EType>(errnum);
}


void PwmError::init()
{
	errnum = invalid_error;
	SensorError::errorMessage = errorMessage.c_str();
}


PwmError::PwmError(const char *message) throw()
	: errorMessage(message)
{
	init();
}


PwmError::PwmError(const std::string &message) throw()
	: errorMessage(message)
{
	init();
}

} /* namespace sensors */
