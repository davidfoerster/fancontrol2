/*
 * subfeature.cpp
 *
 *  Created on: 16.12.2011
 *      Author: malte
 */

#include "subfeature.hpp"
#include "feature.hpp"
#include "chip.hpp"
#include "exceptions.hpp"
#include "util/assert.hpp"
#include <cstring>

#ifndef NDEBUG
#	include <ios>
#	include <cstdlib>
#endif


namespace sensors {


double subfeature::value() const
{
	UTIL_CHECK_POINTER(get());
	UTIL_CHECK_POINTER(parent());
	UTIL_CHECK_POINTER(parent()->parent());

	double v;
	int errnum;
	if ((errnum = sensors_get_value(parent()->parent()->get(), get()->number, &v)) != 0)
		BOOST_THROW_EXCEPTION(sensor_error(errnum));
	return v;
}


void subfeature::value(double v) const
{
	int errnum = sensors_set_value(
			(UTIL_CHECK_POINTER(parent()), UTIL_CHECK_POINTER(parent()->parent())->get()),
			UTIL_CHECK_POINTER(*this)->number,
			v);
	if (errnum != sensor_error::no_error)
		BOOST_THROW_EXCEPTION(sensor_error(errnum));
}


bool subfeature::test_flag(unsigned int flag) const
{
	return (UTIL_CHECK_POINTER(*this)->flags & flag) == flag;
}


subfeature::type_enum subfeature::type_from_name(sensors_feature_type feature, const string_ref &name)
{
	switch (feature) {
		case SENSORS_FEATURE_IN:
		case SENSORS_FEATURE_FAN:
		case SENSORS_FEATURE_TEMP:
		case SENSORS_FEATURE_ENERGY:
		case SENSORS_FEATURE_CURR:
		case SENSORS_FEATURE_HUMIDITY:
			if (name == "input") {
				return static_cast<type_enum>(feature << 8);
			}
			break;

		default:
			break;
	}
	assert_printf(false, "Not (yet) implemented for feature '%s' and subfeature '%s'.",
		feature::Types::name(feature).c_str(), name.c_str());
	return SENSORS_SUBFEATURE_UNKNOWN;
}

} /* namespace sensors */


template std::ostream &operator<<(std::ostream&, const sensors::feature&);
