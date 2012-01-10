/*
 * SensorError.hpp
 *
 *  Created on: 20.12.2011
 *      Author: malte
 */

#pragma once
#ifndef SENSORERROR_HPP_
#define SENSORERROR_HPP_

#include <string>
#include <exception>
#include <climits>

namespace sensors {


class SensorError: public std::exception
{
public:
	enum EType {
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
		invalid_error = INT_MAX
	};

	static EType toEnum(int errnum);

	SensorError(int errnum) throw();

	SensorError(const char *message) throw();

	virtual ~SensorError() throw() {};

	virtual const char* what() const throw() { return errorMessage; }

	int getErrnum() const { return errnum; }

	EType getType() const { return toEnum(errnum); }

protected:
	SensorError() throw() {}

	const char *errorMessage;

	int errnum;
};


class PwmError: public SensorError
{
public:
	PwmError(const char *message) throw();

	PwmError(const std::string &message) throw();

	virtual ~PwmError() throw() {}

protected:
	std::string errorMessage;

private:
	void init();
};

} /* namespace sensors */
#endif /* SENSORERROR_HPP_ */
