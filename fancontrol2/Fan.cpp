/*
 * Fan.cpp
 *
 *  Created on: 22.12.2011
 *      Author: malte
 */

#include "Fan.hpp"
#include "Control.hpp"
#include "sensors++/SubFeature.hpp"
#include "sensors++/Pwm.hpp"

#include <limits>
#include <algorithm>

namespace fancontrol {

typedef Fan::value_t value_t;


Fan::Fan()
	: mDependency(NULL)
	, mMinStart(0.6f)
	, mMinStop(0.5f)
	, mGauge(NULL)
	, mValve(NULL)
	, mLastUpdate(0.0f)
{

}


void Fan::checkGaugeType(const SF *gauge)
	throw(std::invalid_argument)
{
	if (!gauge || gauge->getType() != sensors::SENSORS_SUBFEATURE_FAN_INPUT)
		throw std::invalid_argument("The \"gauge\" must be non-null and a fan input.");
}

void Fan::checkValveType(Pwm *valve)
	throw(std::invalid_argument)
{
	if (!valve || !valve->exists())
		throw std::invalid_argument("The \"valve\" must be non-null and point to a readable and writable file.");
}


double Fan::readGauge() const
{
	return mGauge->getValue();
}


value_t Fan::readValve() const
{
	Pwm::value_t value;
	Pwm::iostate state = mValve->getValue(value);
	return (state == 0) ? static_cast<value_t>(value) : std::numeric_limits<value_t>::signaling_NaN();
}


void Fan::writeValve(value_t value)
{
	mValve->setValue(value);
}


value_t Fan::generateEffectiveValue() const
{
	value_t value = mDependency->getRate();
	if (value > 0 && value < mMinStart && mGauge->getValue() == 0)
		value = mMinStart;
	return value;
}


void Fan::updateValve()
{
	value_t value = generateEffectiveValue();
	writeValve(value);
	mLastUpdate = value;
}


bool Fan::operator==(const Fan &o) const
{
	if (this == &o)
		return true;

	const bool b = *mValve == *o.mValve;
	assert(b == (*mGauge == *o.mGauge && mMinStart == o.mMinStart && mMinStop == o.mMinStop));
	return b;
}

} /* namespace fancontrol */
