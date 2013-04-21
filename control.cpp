/*
 * control.cpp
 *
 *  Created on: 22.12.2011
 *      Author: malte
 */

#define CONTROL_HPP_API
#include "control.hpp"
#include "sensors++/subfeature.hpp"

#include "util/algorithm.hpp"
#include <boost/range/size.hpp>
#include <algorithm>

#include <boost/assert.hpp>


namespace fancontrol {

	using std::shared_ptr;

	typedef control::value_t value_t;
	typedef simple_bounded_control::SF SF;


	control::~control()
	{ }


	bounded_control::~bounded_control()
	{ }


	value_t bounded_control::convert_rate(value_t raw_value) const
	{
		return util::clip<const value_t>((raw_value - m_lower_bound) / (m_upper_bound - m_lower_bound), 0, 1);
	}


	value_t simple_bounded_control::rate_impl() const
	{
		double value = m_source->value();
		value_t rate = (this->*m_rate_converter)(static_cast<value_t>(value));
		//UTIL_LOG(5, "Reading from " << *m_source << ':' << ' ' << value << '=' << '>' << rate);
		return rate;
	}


	const shared_ptr<const SF> &
	simple_bounded_control::check_source_type(const shared_ptr<const SF> &gauge)
		throw(std::invalid_argument)
	{
		if (gauge && !gauge->test_flag(SF::flags::readable))
			BOOST_THROW_EXCEPTION(std::invalid_argument("Unreadable source type"));
		return gauge;
	}


	bool simple_bounded_control::source_comparator::operator()(
			const simple_bounded_control &o, const SF &source
	) const {
		return o.m_source && *o.m_source == source;
	}


	bool simple_bounded_control::source_comparator::operator()(const control &o, const SF &source) const
	{
		const simple_bounded_control *const o_ = dynamic_cast<const simple_bounded_control*>(&o);
		return o_ && operator()(*o_, source);
	}


	simple_bounded_control::simple_bounded_control(
			const shared_ptr<const SF> &source,
			rate_conversion_fun_t rate_converter)
	throw(std::invalid_argument)
		: bounded_control(rate_converter)
		, m_source(check_source_type(source))
	{
	}


	simple_bounded_control::simple_bounded_control(
			const shared_ptr<const SF> &source, value_t lower_bound, value_t upper_bound,
			rate_conversion_fun_t rate_converter)
	throw(std::invalid_argument)
		: bounded_control(lower_bound, upper_bound, rate_converter)
		, m_source(check_source_type(source))
	{
	}


	simple_bounded_control::~simple_bounded_control()
	{
	}


	void simple_bounded_control::source(const shared_ptr<const SF> &source)
	throw(std::invalid_argument)
	{
		m_source = check_source_type(source);
	}


	aggregated_control_base::~aggregated_control_base()
	{ }


	value_t aggregated_control_base::rate_impl() const
	{
		std::pair<aggregated_control_base::const_iterator, aggregated_control_base::const_iterator>
			s = this->sources();
		value_t max;
		if (s.first != s.second) {
			max = (*s.first)->rate();
			while (++s.first != s.second) {
				const value_t v = (*s.first)->rate();
				if (v > max)
					max = v;
			}
		} else {
			max = 0;
		}

		//UTIL_LOG(5, sources.size() << " sources have the aggregated maximum value " << max << '.');

		return max;
	}

} /* namespace fancontrol */
