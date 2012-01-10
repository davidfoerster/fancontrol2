/*
 * SubFeature.cpp
 *
 *  Created on: 16.12.2011
 *      Author: malte
 */

#include "SubFeature.hpp"
#include "Feature.hpp"
#include "Chip.hpp"
#include "SensorError.hpp"
#include <cassert>
#include <cstring>

namespace sensors {


bool SubFeature::operator==(const SubFeature &other) const
{
	const T
		*subfeat = this->object.get(),
		*other_subfeat = other.object.get();
	const bool b = ( subfeat == other_subfeat ||
		(	WrappedTypeComparator::compare(*this, other_subfeat) == 0 &&
			*this->parent == *other.parent &&
			true
		) );
	assert(!b || (subfeat->type == other_subfeat->type && subfeat->mapping == other_subfeat->mapping && std::strcmp(subfeat->name, other_subfeat->name) == 0));
	return b;
}


double SubFeature::getValue() const
{
	assert(get() && parent && parent->getParent() && parent->getParent()->get());

	double v;
	int errnum;
	if ((errnum = sensors_get_value(parent->getParent()->get(), getNumber(), &v)) != 0)
		throw SensorError(errnum);
	return v;
}


void SubFeature::setValue(double v) const
{
	assert(parent && parent->getParent() && parent->getParent()->get() && get());

	int errnum;
	if ((errnum = sensors_set_value(parent->getParent()->get(), getNumber(), v)) != 0)
		throw SensorError(errnum);
}


SubFeature::EType SubFeature::getTypeFromName(sensors_feature_type feature, const string_ref &name)
{
	switch (feature) {
		case SENSORS_FEATURE_IN:
		case SENSORS_FEATURE_FAN:
		case SENSORS_FEATURE_TEMP:
		case SENSORS_FEATURE_ENERGY:
		case SENSORS_FEATURE_CURR:
		case SENSORS_FEATURE_HUMIDITY:
			if (name == "input") {
				return static_cast<EType>(feature << 8);
			}
			break;

		default:
			break;
	}
	return SENSORS_SUBFEATURE_UNKNOWN;
}

} /* namespace sensors */
