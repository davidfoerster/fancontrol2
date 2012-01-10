/*
 * Control.hpp
 *
 *  Created on: 22.12.2011
 *      Author: malte
 */

#pragma once
#ifndef CONTROL_HPP_
#define CONTROL_HPP_

//#include <string>
#include <vector>
#include <stdexcept>
#include <cstddef>

namespace sensors {
class SubFeature;
}

namespace fancontrol {


class Control
{
public:
	typedef float value_t;

	virtual ~Control() {}

	virtual value_t getRate() const = 0;

	//const std::string &getLabel() const { return mLabel; }

protected:
	Control() {}

	//Control(const std::string &label) : mLabel(label) {}

private:
	//std::string mLabel;
};


class BoundedControl: public Control
{
public:
	typedef value_t (BoundedControl::*rate_conversion_fun_t)(value_t) const;

	BoundedControl(rate_conversion_fun_t rateConverter = &BoundedControl::convertRate)
	:	mRateConverter(rateConverter)
	{}

	BoundedControl(value_t lowerBound, value_t upperBound, rate_conversion_fun_t rateConverter = &BoundedControl::convertRate)
	:	mRateConverter(rateConverter)
	,	mLowerBound(lowerBound), mUpperBound(upperBound)
	{}

	value_t convertRate(value_t raw_value) const;

	rate_conversion_fun_t mRateConverter;

	value_t mLowerBound, mUpperBound;
};


class SimpleBoundedControl: public BoundedControl
{
public:
	typedef sensors::SubFeature SF;

	explicit SimpleBoundedControl(const SF &source, rate_conversion_fun_t rateConverter = &BoundedControl::convertRate)
	throw(std::invalid_argument)
	:	BoundedControl(rateConverter)
	{
		setSource(source);
	}

	SimpleBoundedControl(const SF &source, value_t lowerBound, value_t upperBound, rate_conversion_fun_t rateConverter = &BoundedControl::convertRate)
	throw(std::invalid_argument)
	:	BoundedControl(lowerBound, upperBound, rateConverter)
	{
		setSource(source);
	}

	const SF &getSource() const { return *mSource; }

	void setSource(const SF &source) throw(std::invalid_argument) {
		checkSourceType(&source);
		mSource = &source;
	}

	virtual value_t getRate() const;

	bool operator==(const SF &source) const { return mSource == &source; }
	bool operator!=(const SF &source) const { return !operator==(source); }

	struct SourceComparator {
		static ptrdiff_t compare(const SimpleBoundedControl &o, const SF &source);

		static ptrdiff_t compare(const Control &o, const SF &source) {
			const SimpleBoundedControl *o_ = dynamic_cast<const SimpleBoundedControl*>(&o);
			return o_ ? compare(*o_, source) : 1;
		}
	};

private:
	static void checkSourceType(const SF *mGauge) throw(std::invalid_argument);

	const SF *mSource;

	friend struct SourceComparator;
};


class AggregatedControl: public Control
{
public:
	typedef std::vector<const Control*> Container;

	Container sources;

	virtual value_t getRate() const;
};


} /* namespace fancontrol */
#endif /* CONTROL_HPP_ */
