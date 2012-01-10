/*
 * Config.cpp
 *
 *  Created on: 01.01.2012
 *      Author: malte
 */

#include "Config.hpp"
#include "sensors++/sensors.hpp"
#include "sensors++/common.hpp"
#include "meta/algorithm.hpp"
#include <yaml-cpp/yaml.h>
#include <boost/format.hpp>
#include <istream>
#include <cassert>
#include <cmath>
#include <ctime>
#include <cerrno>


namespace fancontrol {

using namespace sensors;
using YAML::Node;
using std::string;


Chip &Config::parseChip(const Node &node) throw(sensors::SensorError, YAML::ParserException)
{
	string name; node["name"] >> name;
	Chip *chip = sensors.getChip(name.c_str());
	if (chip) {
		if (chip->features.empty())
			chip->discoverFeatures();
		return *chip;
	}

	throw SensorError(SensorError::unparsable_chip_name);
}


SubFeature &Config::parseSubFeature(const Node &node) throw(sensors::SensorError, YAML::ParserException)
{
	Chip &chip = parseChip(node["chip"]);

	std::string name_buf; node["input"] >> name_buf;
	string_ref name(name_buf);

	Feature *feat = chip.getFeature(name, true);
	if (feat) {
		SubFeature *sfeat = feat->getSubFeature(name);
		if (sfeat) {
			return *sfeat;
		}
	}

	throw SensorError(SensorError::unparsable_general);
}


Pwm &Config::parsePwm(const Node &node) throw(sensors::SensorError, YAML::ParserException, std::ios::failure)
{
	Chip &chip = parseChip(node["chip"]);

	int idx; node["output"] >> idx;
	errno = 0;
	Pwm *pwm = chip.getPwm(idx);
	if (pwm) {
		return *pwm;
	} else {
		boost::format fmt("No such PWM port: %1% (%2%)");
		fmt % Pwm::makeBasePath(chip, idx) % (errno ? std::strerror(errno) : "<Unknown error>");
		throw PwmError(fmt.str());
	}
}


SimpleBoundedControl &Config::parseSimpleControl(const YAML::Node &node) throw(sensors::SensorError, YAML::ParserException)
{
	SubFeature &source = parseSubFeature(node["source"]);
	ControlsContainer::iterator it_ctrl = meta::find<SimpleBoundedControl::SourceComparator>(controls.begin(), controls.end(), source);
	if (it_ctrl == controls.end()) {
		SimpleBoundedControl::value_t min, max;
		node["min"] >> min;
		node["max"] >> max;
		controls.push_back(std::auto_ptr<Control>(new SimpleBoundedControl(source, min, max)));
		it_ctrl = controls.end() - 1;
	}
	assert(dynamic_cast<SimpleBoundedControl*>(&(*it_ctrl)) != 0);
	return static_cast<SimpleBoundedControl&>(*it_ctrl);
}


AggregatedControl &Config::parseAggregatedControl(const YAML::Node &node) throw(sensors::SensorError, YAML::ParserException)
{
	AggregatedControl &ctrl = *new AggregatedControl();
	ctrl.sources.reserve(node.size());
	for (YAML::Iterator it = node.begin(); it != node.end(); ++it) {
		ctrl.sources.push_back(&parseSimpleControl(*it));
	}
	controls.push_back(&ctrl);
	return ctrl;
}


Control &Config::parseDependencies(const YAML::Node &node) throw(sensors::SensorError, YAML::ParserException)
{
	return (node.Type() == YAML::NodeType::Sequence) ?
			static_cast<Control&>(parseAggregatedControl(node)) :
			static_cast<Control&>(parseSimpleControl(node));
}


Fan &Config::parseFan(const YAML::Node &node) throw(sensors::SensorError, YAML::ParserException, std::ios::failure)
{
	Fan fan;

	node["start"] >> fan.mMinStart;
	Pwm::normalize(&fan.mMinStart);
	node["stop"] >> fan.mMinStop;
	Pwm::normalize(&fan.mMinStop);

	fan.setGauge(parseSubFeature(node["gauge"]));
	fan.setValve(parsePwm(node["valve"]));
	fan.mDependency = &parseDependencies(node["dependencies"]);

	fans.push_back(fan);
	return fans.back();
}


size_t Config::parseFans(const YAML::Node &node) throw(sensors::SensorError, YAML::ParserException, std::ios::failure)
{
	for (YAML::Iterator it = node.begin(); it != node.end(); ++it) {
		Fan &fan = parseFan(it.second());
		it.first() >> fan.mLabel.value;
	}
	return node.size();
}


void Config::getInterval(struct timespec *t) const
{
	double seconds;
	t->tv_nsec = static_cast<long>(std::modf(interval, &seconds) * 1e+9);
	t->tv_sec = static_cast<time_t>(seconds);
}


Config::Config(std::istream &source, Sensors &sensors) throw(sensors::SensorError, YAML::ParserException, std::ios::failure)
	: sensors(sensors)
{
	YAML::Parser parser(source);
	YAML::Node doc;
	while (parser.GetNextDocument(doc) && doc.Type() != YAML::NodeType::Null) {
		const YAML::Node &nInterval = doc["interval"];
		if (nInterval.Type() != YAML::NodeType::Null) {
			nInterval >> interval;
			assert(interval > 0);
		} else {
			interval = 10;
		}

		parseFans(doc["fans"]);
	}
}


} /* namespace fancontrol */
