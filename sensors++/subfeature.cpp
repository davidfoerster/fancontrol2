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

#ifndef NDEBUG
#	include <ios>
#	include <cstdlib>
#endif

#include "meta/assert.hpp"
#include <cstring>

namespace sensors {

using boost::shared_ptr;


double subfeature::value() const throw(sensor_error)
{
	META_CHECK_POINTER(get());
	META_CHECK_POINTER(parent());
	META_CHECK_POINTER(parent()->parent());

	double v;
	int errnum;
	if ((errnum = sensors_get_value(parent()->parent()->get(), get()->number, &v)) != 0)
		BOOST_THROW_EXCEPTION(sensor_error(errnum));
	return v;
}


void subfeature::value(double v) const
	throw(meta::null_pointer_exception, sensor_error)
{
	int errnum = sensors_set_value(
			(META_CHECK_POINTER(parent()), META_CHECK_POINTER(parent()->parent())->get()),
			META_CHECK_POINTER(*this)->number,
			v);
	if (errnum != sensor_error::no_error)
		BOOST_THROW_EXCEPTION(sensor_error(errnum));
}


bool subfeature::test_flag(unsigned int flag) const
{
	return (META_CHECK_POINTER(*this)->flags & flag) == flag;
}


bool subfeature::operator==(const super &o) const
{
	return super::operator==(o);
}


bool subfeature::operator<(const super &o) const
{
	return super::operator<(o);
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
			if (name == string_ref::make_const("input")) {
				return static_cast<type_enum>(feature << 8);
			}
			break;

		default:
			break;
	}
	assert_printf(false, "Not (yet) implemented for feature '%s' and subfeature '%s'.", feature::Types::name(feature).data(), name.data());
	return SENSORS_SUBFEATURE_UNKNOWN;
}


template std::ostream &operator<<(std::ostream &out, const subfeature &sfeat);

} /* namespace sensors */
