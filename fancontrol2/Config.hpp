/*
 * Config.hpp
 *
 *  Created on: 01.01.2012
 *      Author: malte
 */

#pragma once
#ifndef FANCONTROL_CONFIG_HPP_
#define FANCONTROL_CONFIG_HPP_

#include "Fan.hpp"
#include "Control.hpp"
#include "sensors++/SensorError.hpp"
#include <yaml-cpp/exceptions.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include <vector>


struct timespec;

namespace std {

template<class> struct char_traits;
template<typename, typename> class basic_istream;
typedef basic_istream< char, char_traits<char> > istream;

}

namespace YAML {

class Node;

}

namespace sensors {

class Sensors;
class Chip;
class Pwm;
class SubFeature;

}


namespace fancontrol {

class Fan;
class SimpleBoundedControl;
class AggregatedControl;


class Config
{
public:
	Config(std::istream &source, sensors::Sensors &sensors) throw(sensors::SensorError, YAML::ParserException, std::ios::failure);

	double interval;

	double getInterval() const { return interval; }
	void getInterval(struct timespec *t) const;

	sensors::Sensors &sensors;

	typedef boost::ptr_vector<Control> ControlsContainer;
	ControlsContainer controls;

	std::vector<Fan> fans;

private:
	sensors::Chip &parseChip(const YAML::Node &node) throw(sensors::SensorError, YAML::ParserException);

	sensors::SubFeature &parseSubFeature(const YAML::Node &node) throw(sensors::SensorError, YAML::ParserException);

	sensors::Pwm &parsePwm(const YAML::Node &node) throw(sensors::SensorError, YAML::ParserException, std::ios::failure);

	SimpleBoundedControl &parseSimpleControl(const YAML::Node &node) throw(sensors::SensorError, YAML::ParserException);

	AggregatedControl &parseAggregatedControl(const YAML::Node &node) throw(sensors::SensorError, YAML::ParserException);

	Control &parseDependencies(const YAML::Node &node) throw(sensors::SensorError, YAML::ParserException);

	Fan &parseFan(const YAML::Node &node) throw(sensors::SensorError, YAML::ParserException, std::ios::failure);

	size_t parseFans(const YAML::Node &node) throw(sensors::SensorError, YAML::ParserException, std::ios::failure);
};

} /* namespace fancontrol */
#endif /* FANCONTROL_CONFIG_HPP_ */
