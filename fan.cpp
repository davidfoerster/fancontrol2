/*
 * fan.cpp
 *
 *  Created on: 22.12.2011
 *      Author: malte
 */

#include "fan.hpp"
#include "control.hpp"
#include "sensors++/subfeature.hpp"
#include "sensors++/pwm.hpp"

#include <boost/assert.hpp>

namespace fancontrol {

using boost::shared_ptr;

using sensors::sensor_error;
using sensors::io_error;
using sensors::subfeature;
using sensors::pwm;

typedef fan::value_t value_t;


fan::fan()
	: m_min_start(0.6f)
	, m_max_stop(0.5f)
	, m_reset_rate(1.0f)
	, m_last_update(0.0f)
{
}


bool fan::gauge_type_guard::check(const SF *gauge)
{
	return gauge && gauge->get() && gauge->get()->type == sensors::SENSORS_SUBFEATURE_FAN_INPUT;
}


double fan::gauge_wrapper::read() const throw (sensor_error)
{
	return m_value->value();
}


bool fan::valve_type_guard::check(const pwm *valve)
{
	return valve && valve->exists();
}


value_t fan::valve_wrapper::read() const throw(io_error)
{
	return m_value->value();
}


void fan::valve_wrapper::write(value_t value) throw(io_error)
{
	m_value->value(value);
}


value_t fan::effective_value(value_t value) const
{
	BOOST_ASSERT(value >= 0);
	if (value > 0 && value < m_min_start) {
		if (m_gauge.read() == 0) {
			value = m_min_start;
		} else if (value < m_max_stop) {
			value = m_max_stop;
		}
	}
	return value;
}


void fan::update_valve(value_t value)
{
	m_valve.write(value);
	m_last_update = value;
}


void fan::update_valve()
{
	update_valve(effective_value(META_CHECK_POINTER(m_dependency)->rate()));
}


void fan::reset()
{
	update_valve(effective_value(m_reset_rate));
}


bool fan::operator==(const fan &o) const
{
	if (this == &o)
		return true;

	const bool b = **m_valve == **o.m_valve;
	BOOST_ASSERT(!b || (**m_gauge == **o.m_gauge && m_min_start == o.m_min_start && m_max_stop == o.m_max_stop));
	return b;
}

} /* namespace fancontrol */
