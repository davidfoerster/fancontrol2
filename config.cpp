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

#include "util/strcat.hpp"
#include "util/algorithm.hpp"
#include "util/yaml.hpp"
#include "util/static_allocator/static_string.hpp"

#include <boost/format.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <yaml-cpp/yaml.h>

#include <string>
#include <sstream>
#include <istream>
#include <iostream>
#include <functional>

#include <boost/assert.hpp>
#include <cmath>
#include <ctime>
#include <cerrno>


#ifndef FANCONTROL_PIDFILE
#	define FANCONTROL_PIDFILE (1)
#endif
#if FANCONTROL_PIDFILE
#	ifndef FANCONTROL_PIDFILE_ROOTONLY
#		ifdef NDEBUG
#			define FANCONTROL_PIDFILE_ROOTONLY (1)
#		else
#			define FANCONTROL_PIDFILE_ROOTONLY (0)
#		endif
#	endif
#	ifndef FANCONTROL_PIDFILE_PATH
#		if FANCONTROL_PIDFILE_ROOTONLY
#			define FANCONTROL_PIDFILE_PATH /var/run/fancontrol.pid
#		else
#			define FANCONTROL_PIDFILE_PATH fancontrol.pid
#		endif
#	endif
#endif


namespace fancontrol {

	using std::string;
	using std::istream;

	using std::shared_ptr;
	using std::make_shared;
	using std::static_pointer_cast;
	using std::bind;
	using std::ref;
	using std::cref;
	using namespace std::placeholders;

	using YAML::ParserException;
	using YAML::Node;
	using YAML::NodeType;

	using namespace sensors;

	typedef config::ios_failure ios_failure;

	typedef util::static_string<24> name_buffer_type;


	shared_ptr<chip>
	config::parse_chip(const Node &node)
			throw(sensor_error, ParserException)
	{
		name_buffer_type name;
		node["name"] >> name;
		shared_ptr<chip> chip(sensors->chip(name));
		if (chip)
			return chip;

		BOOST_THROW_EXCEPTION(sensor_error(sensor_error::unparsable_chip_name) << sensor_error::chip_name(name));
	}


	shared_ptr<subfeature>
	config::parse_subfeature(const Node &node)
			throw(sensor_error, ParserException)
	{
		shared_ptr<chip> chip(parse_chip(node["chip"]));

		name_buffer_type name_buf;
		node["input"] >> name_buf;
		string_ref name(name_buf);

		shared_ptr<feature> feat(chip->feature_consume_name(name));
		if (feat) {
			shared_ptr<subfeature> sfeat(feat->subfeature(name));
			if (sfeat) {
				return sfeat;
			}
		}

		sensor_error e("No such subfeature");
		e << sensor_error::chip_name(chip->prefix().str());
		e << sensor_error::feature_name(name_buf);
		if (feat) e << sensor_error::subfeature_name(name.str());
		BOOST_THROW_EXCEPTION(e);
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

		std::ostringstream feature_name;
		feature_name << pwm::Item::prefix() << idx;
		throw pwm_error("No such PWM port")
				<< sensor_error::chip_name(chip->prefix().str())
				<< sensor_error::feature_name(feature_name.str())
				<< io_error::errno_code(errno);
	}


	shared_ptr<control>
	config::parse_simple_control(const Node &node)
			throw(sensor_error, ParserException)
	{
		shared_ptr<subfeature> source(parse_subfeature(node["source"]));
		controls_container::const_iterator it_ctrl = boost::find_if(controls,
				bind(simple_bounded_control::source_comparator(), _1, cref(*source)));

		if (it_ctrl == controls.end()) {
			simple_bounded_control::value_t min, max;
			node["min"] >> min;
			node["max"] >> max;
			controls.push_back(static_pointer_cast<control>(
					make_shared<simple_bounded_control>(source, min, max)));
			it_ctrl = controls.end() - 1;
		}
		return *it_ctrl;
	}


	shared_ptr<control>
	config::parse_aggregated_control(const Node &node)
			throw(sensor_error, ParserException)
	{
		if (node.size() != 0) {
			typedef boost::transform_iterator<
						std::function<shared_ptr<control>(const Node&)>,
						YAML::Iterator
				> parse_dependency_iterator;

			controls.push_back(static_pointer_cast<control>(
					make_shared< aggregated_control<> >(
						parse_dependency_iterator(node.begin(), bind(&config::parse_dependencies, ref(*this), _1)),
						parse_dependency_iterator(node.end()),
					node.size())));
			return controls.back();

		} else {
			return shared_ptr<control>();
		}
	}


	shared_ptr<control>
	config::parse_dependencies(const Node &node)
			throw(sensor_error, ParserException)
	{
		return (node.Type() == NodeType::Sequence) ?
				parse_aggregated_control(node) :
				parse_simple_control(node);
	}


	const shared_ptr<fan> &
	config::parse_fan(const Node &node)
			throw(sensor_error, ParserException, ios_failure)
	{
		shared_ptr<fan> fan(make_shared<fan>());

		node["start"] >> fan->m_min_start;
		node["stop"] >> fan->m_max_stop;

		const Node &reset_rate = node["reset"];
		if (reset_rate.Type() != NodeType::Null)
			reset_rate >> fan->m_reset_rate;

		fan->m_gauge.m_value = parse_subfeature(node["gauge"]);

		fan->m_valve.m_value = parse_pwm(node["valve"]);

		fan->m_dependency = parse_dependencies(node["dependencies"]);

		fans.push_back(fan);
		return fans.back();
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
			throw(util::runtime_error, ParserException, ios_failure)
		: auto_reset(true)
		, sensors(sensors)
	{
		if (!do_check) {
	#if FANCONTROL_PIDFILE
			m_pidfile.reset(new util::pidfile(
					string_ref(BOOST_PP_STRINGIZE(FANCONTROL_PIDFILE_PATH)),
					FANCONTROL_PIDFILE_ROOTONLY));
#endif
		}

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
