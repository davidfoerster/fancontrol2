/*
 * Fan.hpp
 *
 *  Created on: 22.12.2011
 *      Author: malte
 */

#pragma once
#ifndef FANCONTROL_FAN_HPP_
#define FANCONTROL_FAN_HPP_

#include <string>
#include <stdexcept>

namespace sensors {
class SubFeature;
class Pwm;
}

namespace fancontrol {

class Control;
class Config;


template <class T>
class PropertyWrapper {
protected:
	T value;

public:
	explicit PropertyWrapper(const T &value) : value(value) {}

	PropertyWrapper() {}

	const T &get() const { return value; }
};


class Fan
{
public:
	typedef float value_t;

	typedef sensors::Pwm Pwm;
	typedef sensors::SubFeature SF;

	Fan();

	const SF &getGauge() const { return *mGauge; }

	void setGauge(const SF &gauge) throw(std::invalid_argument) {
		checkGaugeType(&gauge);
		mGauge = &gauge;
	}

	double readGauge() const;

	Pwm &getValve() const { return *mValve; }

	void setValve(Pwm &valve) throw(std::invalid_argument) {
		checkValveType(&valve);
		mValve = &valve;
	}

	value_t readValve() const;

	void writeValve(value_t value);

	value_t generateEffectiveValue() const;

	void updateValve();

	bool operator==(const Fan &o) const;
	bool operator!=(const Fan &o) const { return !operator==(o); }

	const std::string &getLabel() const { return mLabel.get(); }

	const Control *mDependency;

	value_t mMinStart, mMinStop;

	class LabelWrapper: public PropertyWrapper<std::string> {
	public:
		bool set(const std::string &str) {
			if (value.empty()) {
				value = str;
				return true;
			}
			return false;
		}

		friend class Config;
	}
	mLabel;

private:
	static void checkGaugeType(const SF *gauge) throw(std::invalid_argument);

	static void checkValveType(Pwm *valve) throw(std::invalid_argument);

	const SF *mGauge;

	Pwm *mValve;

	value_t mLastUpdate;
};

} /* namespace fancontrol */
#endif /* FANCONTROL_FAN_HPP_ */
