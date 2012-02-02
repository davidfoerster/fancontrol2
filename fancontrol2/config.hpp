/*
 * config.hpp
 *
 *  Created on: 01.01.2012
 *      Author: malte
 */

#pragma once
#ifndef FANCONTROL_CONFIG_HPP_
#define FANCONTROL_CONFIG_HPP_

#include "meta/pidfile.hpp"

#include "sensors++/exceptions.hpp"
#include <yaml-cpp/exceptions.h>

#include <boost/shared_ptr.hpp>
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

class sensor_container;
class chip;
class pwm;
class subfeature;

}


namespace fancontrol {

using std::istream;

using boost::shared_ptr;
using YAML::ParserException;
using YAML::Node;

using sensors::sensor_error;
using sensors::sensor_container;
using sensors::chip;
using sensors::pwm;
using sensors::subfeature;

class fan;
class control;
class simple_bounded_control;
class aggregated_control;


class config
{
public:
	typedef std::ios::failure ios_failure;

	config(istream &source, const shared_ptr<sensor_container> &sensors, bool do_check = false)
			throw(meta::runtime_error, ParserException, ios_failure);

	~config();

	bool auto_reset;

	void reset();

	double m_interval;

	double interval() const { return m_interval; }
	void interval(struct timespec *t) const;

	shared_ptr<sensor_container> sensors;

	typedef std::vector< shared_ptr<control> > controls_container;
	controls_container controls;

	typedef std::vector< shared_ptr<fan> > fans_container;
	fans_container fans;

private:
	shared_ptr<chip> parse_chip(const Node &node)
			throw(sensor_error, ParserException);

	shared_ptr<subfeature> parse_subfeature(const Node &node)
			throw(sensor_error, ParserException);

	shared_ptr<pwm> parse_pwm(const Node &node)
			throw(sensor_error, ParserException, ios_failure);

	const shared_ptr<control> &parse_simple_control(const Node &node)
			throw(sensor_error, ParserException);

	const shared_ptr<control> &parse_aggregated_control(const Node &node)
			throw(sensor_error, ParserException);

	const shared_ptr<control> &parse_dependencies(const Node &node)
			throw(sensor_error, ParserException);

	shared_ptr<fan> parse_fan(const Node &node)
			throw(sensor_error, ParserException, ios_failure);

	fans_container::size_type parse_fans(const Node &node)
			throw(sensor_error, ParserException, ios_failure);

	std::auto_ptr<meta::pidfile> m_pidfile;
};

} /* namespace fancontrol */
#endif /* FANCONTROL_CONFIG_HPP_ */
