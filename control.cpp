/*
 * control.cpp
 *
 *  Created on: 22.12.2011
 *      Author: malte
 */

#include "control.hpp"
#include "sensors++/subfeature.hpp"

#include "meta/algorithm.hpp"
#include <algorithm>

#include <boost/assert.hpp>

namespace fancontrol {

using boost::shared_ptr;

typedef control::value_t value_t;
typedef simple_bounded_control::SF SF;


value_t bounded_control::convert_rate(value_t raw_value) const
{
	return meta::clip<value_t>((raw_value - m_lower_bound) / (m_upper_bound - m_lower_bound), 0, 1);
}


value_t simple_bounded_control::rate_impl() const
{
	return (this->*m_rate_converter)(static_cast<value_t>(m_source->value()));
}


const shared_ptr<const SF> &
simple_bounded_control::check_source_type(const shared_ptr<const SF> &gauge)
	throw(std::invalid_argument)
{
	if (gauge && !gauge->test_flag(SF::flags::readable))
		throw std::invalid_argument("Unreadable source type");
	return gauge;
}


int simple_bounded_control::source_comparator::compare(
		const simple_bounded_control &o, const SF &source
) {
	return o.m_source ? meta::compare(*o.m_source, source) : 1;
}


int simple_bounded_control::source_comparator::compare(const control &o, const SF &source)
{
	const simple_bounded_control *o_ = dynamic_cast<const simple_bounded_control*>(&o);
	return o_ ? compare(*o_, source) : 1;
}


int simple_bounded_control::source_comparator::compare(
		const shared_ptr<const control> &o, const SF &source)
{
	return o ? compare(*o, source) : 1;
}


value_t aggregated_control::rate_impl() const
{
	container_type::const_iterator it(sources.begin());
	value_t max;
	if (it != sources.end()) {
		max = (*it)->rate();
		while (++it != sources.end()) {
			value_t v = (*it)->rate();
			if (v > max)
				max = v;
		}
	} else {
		max = 0;
	}
	return max;
}

} /* namespace fancontrol */
