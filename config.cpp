/*
 * config.cpp
 *
 *  Created on: 01.01.2012
 *      Author: malte
 */

#include "config.hpp"
#include "control.hpp"
#include "fan.hpp"

#include "sensors++/sensors.hpp"
#include "sensors++/chip.hpp"
#include "sensors++/feature.hpp"
#include "sensors++/subfeature.hpp"
#include "sensors++/pwm.hpp"

#include "meta/strcat.hpp"
#include "meta/algorithm.hpp"
#include "meta/utility.hpp"

#include <boost/make_shared.hpp>
#include <boost/format.hpp>
#include <yaml-cpp/yaml.h>

#include <string>
#include <istream>
#include <iostream>

#include <boost/assert.hpp>
#include <cmath>
#include <ctime>
#include <cerrno>


#if FANCONTROL_PIDFILE
#	ifndef FANCONTROL_PIDFILE_PATH
#		define FANCONTROL_PIDFILE_PATH /var/run/fancontrol.pid
#	endif
#	ifndef FANCONTROL_PIDFILE_ROOTONLY
#		define FANCONTROL_PIDFILE_ROOTONLY (1)
#	endif
#endif


namespace fancontrol {

using std::string;
using std::istream;

using boost::shared_ptr;
using boost::make_shared;
using boost::static_pointer_cast;

using YAML::ParserException;
using YAML::Node;
using YAML::NodeType;

using namespace sensors;

typedef config::ios_failure ios_failure;


shared_ptr<chip>
config::parse_chip(const Node &node)
		throw(sensor_error, ParserException)
{
	string name; node["name"] >> name;
	shared_ptr<chip> chip(sensors->chip(name));
	if (chip)
		return chip;

	throw sensor_error(sensor_error::unparsable_chip_name)
			<< sensor_error::chip_name(name);
}


shared_ptr<subfeature>
config::parse_subfeature(const Node &node)
		throw(sensor_error, ParserException)
{
	shared_ptr<chip> chip(parse_chip(node["chip"]));

	string name_buf; node["input"] >> name_buf;
	string_ref name(name_buf);

	shared_ptr<feature> feat(chip->feature_consume_name(name));
	if (feat) {
		shared_ptr<subfeature> sfeat(feat->subfeature(name));
		if (sfeat) {
			return sfeat;
		}
	}

	sensor_error e("No such subfeature");
	e << sensor_error::chip_name(chip->prefix().as_string());
	e << sensor_error::feature_name(name_buf);
	if (feat) e << sensor_error::subfeature_name(name.as_string());
	throw e;
}


shared_ptr<pwm>
config::parse_pwm(const Node &node)
		throw(sensor_error, ParserException, ios_failure)
{
	shared_ptr<chip> chip(parse_chip(node["chip"]));

	int idx; node["output"] >> idx;
	errno = 0;
	shared_ptr<pwm> pwm(chip->pwm(idx));
	if (pwm)
		return pwm;

	throw pwm_error("No such PWM port")
			<< sensor_error::chip_name(chip->prefix().as_string())
			<< sensor_error::feature_name((meta::strcat() << pwm::Item::prefix() << idx)())
			<< io_error::errno_code(errno);
}


const shared_ptr<control> &
config::parse_simple_control(const Node &node)
		throw(sensor_error, ParserException)
{
	shared_ptr<subfeature> source(parse_subfeature(node["source"]));
	controls_container::iterator it_ctrl =
			meta::find<simple_bounded_control::source_comparator>(controls.begin(), controls.end(), *source);
	if (it_ctrl == controls.end()) {
		simple_bounded_control::value_t min, max;
		node["min"] >> min;
		node["max"] >> max;
		controls.push_back(make_shared<simple_bounded_control>(source, min, max));
		it_ctrl = controls.end() - 1;
	}
	//assert(dynamic_cast<simple_bounded_control*>(it_ctrl->get()));
	return *it_ctrl;
}


const shared_ptr<control> &
config::parse_aggregated_control(const Node &node)
		throw(sensor_error, ParserException)
{
	shared_ptr<aggregated_control> ctrl(make_shared<aggregated_control>());
	ctrl->sources.reserve(node.size());
	for (YAML::Iterator it = node.begin(); it != node.end(); ++it) {
		ctrl->sources.push_back(parse_simple_control(*it));
	}
	controls.push_back(ctrl);
	return controls.back();
}


const shared_ptr<control> &
config::parse_dependencies(const Node &node)
		throw(sensor_error, ParserException)
{
	return (node.Type() == NodeType::Sequence) ?
			parse_aggregated_control(node) :
			parse_simple_control(node);
}


shared_ptr<fan>
config::parse_fan(const Node &node)
		throw(sensor_error, ParserException, ios_failure)
{
	shared_ptr<fan> fan(make_shared<fan>());

	node["start"] >> fan->m_min_start;
	pwm::normalize(&fan->m_min_start);
	node["stop"] >> fan->m_max_stop;
	pwm::normalize(&fan->m_max_stop);

	const Node &reset_rate = node["reset"];
	if (reset_rate.Type() != NodeType::Null)
		reset_rate >> fan->m_reset_rate;

	fan->m_gauge.m_value = parse_subfeature(node["gauge"]);

	fan->m_valve.m_value = parse_pwm(node["valve"]);

	fan->m_dependency = parse_dependencies(node["dependencies"]);

	fans.push_back(fan);
	return fan;
}


config::fans_container::size_type
config::parse_fans(const Node &node)
		throw(sensor_error, ParserException, ios_failure)
{
	for (YAML::Iterator it = node.begin(); it != node.end(); ++it) {
		shared_ptr<fan> fan(parse_fan(it.second()));
		it.first() >> fan->m_label.m_value;
	}
	return node.size();
}


void config::interval(struct timespec *t) const
{
	double seconds;
	t->tv_nsec = static_cast<long>(std::modf(m_interval, &seconds) * 1e+9);
	t->tv_sec = static_cast<time_t>(seconds);
}


config::config(istream &source, const shared_ptr<sensor_container> &sensors, bool do_check)
		throw(meta::runtime_error, ParserException, ios_failure)
	: auto_reset(true)
	, sensors(sensors)
{
#if FANCONTROL_PIDFILE
	if (!do_check) {
		m_pidfile.reset(new meta::pidfile(
				string_ref::make_const(META_STRING(FANCONTROL_PIDFILE_PATH)),
				FANCONTROL_PIDFILE_ROOTONLY, false));
	}
#endif

	YAML::Parser parser(source);
	Node doc;
	while (parser.GetNextDocument(doc) && doc.Type() != NodeType::Null) {
		const Node &interval_node = doc["interval"];
		if (interval_node.Type() != NodeType::Null) {
			interval_node >> m_interval;
			BOOST_ASSERT(m_interval > 0);
		} else {
			m_interval = 10;
		}

		parse_fans(doc["fans"]);
	}
}


config::~config()
{
	if (auto_reset)
		reset_nothrow();
}


void config::reset()
{
	for (fans_container::iterator it(fans.begin()); it != fans.end(); ++it) {
		(*it)->reset();
	}
}


void config::reset_nothrow()
{
	for (fans_container::iterator it(fans.begin()); it != fans.end(); ++it) {
		if (*it) try {
			(*it)->reset();
		} catch (ios_failure &e) {
			std::clog << "Could not reset PWM: " << (*it)->m_valve.get()->path() << std::endl;
		}
	}
}


} /* namespace fancontrol */
