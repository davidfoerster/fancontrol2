/*
 * config.hpp
 *
 *  Created on: 01.01.2012
 *      Author: malte
 */

#pragma once
#ifndef FANCONTROL_CONFIG_HPP_
#define FANCONTROL_CONFIG_HPP_


#ifndef FANCONTROL_PIDFILE
#	define FANCONTROL_PIDFILE (1)
#endif

#if FANCONTROL_PIDFILE
#	include "util/pidfile.hpp"
#endif

#include "sensors++/exceptions.hpp"
#include "util/ptr_wrapper.hpp"
#include "util/static_allocator/static_vector.hpp"
#include "util/memory.hpp"
#include <yaml-cpp/exceptions.h>

#include <memory>
#include <vector>
#include <iosfwd>


struct timespec;

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

	using util::shared_ptr;
	using YAML::ParserException;
	using YAML::Node;

	using sensors::sensor_error;
	using sensors::sensor_container;
	using sensors::chip;
	using sensors::pwm;
	using sensors::subfeature;

	class fan;
	class control;


	class config
	{
	public:
		typedef std::ios::failure ios_failure;

		config(istream &source, const shared_ptr<sensor_container> &sensors, bool do_check = false);

		~config();

		void reset();

		bool auto_reset;

		double m_interval;

		double interval() const;
		void interval(struct timespec *t) const;

		shared_ptr<sensor_container> sensors;

		typedef util::ptr_wrapper< shared_ptr<control> > control_type;
		typedef util::static_vector<control_type, 16> controls_container;
		controls_container controls;

		typedef shared_ptr<fan> fan_type;
		typedef util::static_vector<fan_type, 8> fans_container;
		fans_container fans;

	private:
		shared_ptr<chip> parse_chip(const Node &node);

		shared_ptr<subfeature> parse_subfeature(const Node &node);

		shared_ptr<pwm> parse_pwm(const Node &node);

		shared_ptr<control> parse_simple_control(const Node &node);

		shared_ptr<control> parse_aggregated_control(const Node &node);

		shared_ptr<control> parse_dependencies(const Node &node);

		const shared_ptr<fan> &parse_fan(const Node &node);

		fans_container::size_type parse_fans(const Node &node);

		void reset_nothrow();

#if FANCONTROL_PIDFILE
		std::unique_ptr< util::pidfile > m_pidfile;
#endif

	};



// implementation =============================================================

	inline double config::interval() const
	{
		return m_interval;
	}

} /* namespace fancontrol */
#endif /* FANCONTROL_CONFIG_HPP_ */
