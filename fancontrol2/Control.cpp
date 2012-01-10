/*
 * Control.cpp
 *
 *  Created on: 22.12.2011
 *      Author: malte
 */

#include "Control.hpp"
#include "sensors++/SubFeature.hpp"
#include <algorithm>
#include <cassert>

namespace fancontrol {

typedef Control::value_t value_t;


template <typename T>
T clip(T x, T min, T max)
{
	return std::min<T>(std::max<T>(x, min), max);
}


value_t BoundedControl::convertRate(value_t raw_value) const
{
	return clip<value_t>((raw_value - mLowerBound) / (mUpperBound - mLowerBound), 0, 1);
}


value_t SimpleBoundedControl::getRate() const
{
	return (this->*mRateConverter)(static_cast<value_t>(mSource->getValue()));
}


void SimpleBoundedControl::checkSourceType(const SF *mGauge)
	throw(std::invalid_argument)
{
	if (mGauge && !mGauge->testFlag(SF::Flags::readable))
		throw std::invalid_argument("Unreadable source type");
}


ptrdiff_t SimpleBoundedControl::SourceComparator::compare(const SimpleBoundedControl &o, const SF &source)
{
	return o.mSource - &source;
}


value_t AggregatedControl::getRate() const
{
	Container::const_iterator
		begin(sources.begin()),
		end(sources.end());
	value_t max;

	if (begin != end) {
		assert(*begin);
		for (max = (*begin)->getRate(), ++begin; begin != end; ++begin) {
			assert(*begin);
			value_t value = (*begin)->getRate();
			if (value > max)
				max = value;
		}
	} else {
		max = 1;
	}

	return max;
}

} /* namespace fancontrol */
