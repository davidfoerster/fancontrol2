/*
 * control.hpp
 *
 *  Created on: 22.12.2011
 *      Author: malte
 */

#pragma once
#ifndef CONTROL_HPP_
#define CONTROL_HPP_

#include "meta/algorithm.hpp"

#include <boost/shared_ptr.hpp>
#include <vector>
#include <stdexcept>

#include <boost/assert.hpp>

namespace sensors {
class subfeature;
}

namespace fancontrol {

using boost::shared_ptr;


class control
{
public:
	typedef float value_t;

	virtual ~control();

	value_t rate() const;

	value_t last_rate() const;

	struct rate_comparator {
		static bool less(const control &a, const control &b);

		static int compare(const control &a, const control &b);

		static int compare(const control *a, const control *b);

		static bool less(
				const boost::shared_ptr<control> &a,
				const boost::shared_ptr<control> &b);

		static bool less1(
				const boost::shared_ptr<control> &a,
				const boost::shared_ptr<control> &b);

		static int compare(
				const boost::shared_ptr<control> &a,
				const boost::shared_ptr<control> &b);
	};

protected:
	control();

	virtual value_t rate_impl() const = 0;

private:
	value_t m_last_rate;
};


class bounded_control: public control
{
public:
	typedef value_t (bounded_control::*rate_conversion_fun_t)(value_t) const;

	bounded_control(rate_conversion_fun_t rate_converter = &bounded_control::convert_rate);

	bounded_control(value_t lower_bound, value_t upper_bound,
			rate_conversion_fun_t rate_converter = &bounded_control::convert_rate);

	virtual ~bounded_control();

	value_t convert_rate(value_t raw_value) const;

	rate_conversion_fun_t m_rate_converter;

	value_t m_lower_bound, m_upper_bound;
};


class simple_bounded_control: public bounded_control
{
public:
	typedef sensors::subfeature SF;

	explicit simple_bounded_control(
			const shared_ptr<const SF> &source,
			rate_conversion_fun_t rate_converter = &bounded_control::convert_rate)
		throw(std::invalid_argument);

	simple_bounded_control(
			const shared_ptr<const SF> &source, value_t lower_bound, value_t upper_bound,
			rate_conversion_fun_t rate_converter = &bounded_control::convert_rate)
		throw(std::invalid_argument);

	virtual ~simple_bounded_control();

	const shared_ptr<const SF> &source() const;

	void source(const shared_ptr<const SF> &source) throw(std::invalid_argument);

	bool operator==(const simple_bounded_control &o) const;

	bool operator==(const control &o) const;

	bool operator==(const SF &source) const;

	struct source_comparator {
		static int compare(const simple_bounded_control &o, const SF &source);

		static int compare(const control &o, const SF &source);

		static int compare(const shared_ptr<const control> &o, const SF &source);
	};

protected:
	virtual value_t rate_impl() const;

private:
	static const shared_ptr<const SF> &check_source_type(const shared_ptr<const SF> &m_gauge)
			throw(std::invalid_argument);

	shared_ptr<const SF> m_source;

	friend struct source_comparator;
};


class aggregated_control: public control
{
public:
	typedef std::vector<shared_ptr<control> > container_type;

	virtual ~aggregated_control();

	container_type sources;

protected:
	virtual value_t rate_impl() const;
};



// implementation =============================================================

inline
control::control()
{
}


inline
control::~control()
{
}


inline
control::value_t control::last_rate() const
{
	return m_last_rate;
}


inline
control::value_t control::rate() const
{
	return (const_cast<control*>(this)->m_last_rate = this->rate_impl());
}


inline
bool control::rate_comparator::less(const control &a, const control &b)
{
	return compare(a, b) < 0;
}


inline
int control::rate_comparator::compare(const control &a, const control &b)
{
	return meta::signum(a.rate() - b.rate());
}


inline
int control::rate_comparator::compare(const control *a, const control *b)
{
	return (a && b) ? compare(*a, *b) : a ? 1 : b ? -1 : 0;
}


inline
bool control::rate_comparator::less(
		const shared_ptr<control> &a,
		const shared_ptr<control> &b
) {
	return compare(a, b) < 0;
}


inline
bool control::rate_comparator::less1(
		const shared_ptr<control> &a,
		const shared_ptr<control> &b
) {
	return less(a, b);
}


inline
int control::rate_comparator::compare(
		const shared_ptr<control> &a,
		const shared_ptr<control> &b)
{
	return compare(a.get(), b.get());
}


inline
bounded_control::bounded_control(rate_conversion_fun_t rate_converter)
	: m_rate_converter(rate_converter)
{
}


inline
bounded_control::bounded_control(
		value_t lower_bound, value_t upper_bound,
		rate_conversion_fun_t rate_converter)
	: m_rate_converter(rate_converter)
	, m_lower_bound(lower_bound), m_upper_bound(upper_bound)
{
}


inline
bounded_control::~bounded_control()
{
}


inline
simple_bounded_control::simple_bounded_control(
		const shared_ptr<const SF> &source,
		rate_conversion_fun_t rate_converter)
throw(std::invalid_argument)
	: bounded_control(rate_converter)
	, m_source(check_source_type(source))
{
}


inline
simple_bounded_control::simple_bounded_control(
		const shared_ptr<const SF> &source, value_t lower_bound, value_t upper_bound,
		rate_conversion_fun_t rate_converter)
throw(std::invalid_argument)
	: bounded_control(lower_bound, upper_bound, rate_converter)
	, m_source(check_source_type(source))
{
}


inline
simple_bounded_control::~simple_bounded_control()
{
}


inline
const shared_ptr<const simple_bounded_control::SF> &
simple_bounded_control::source() const
{
	BOOST_ASSERT(m_source);
	return m_source;
}


inline
void simple_bounded_control::source(const shared_ptr<const SF> &source)
throw(std::invalid_argument)
{
	m_source = check_source_type(source);
}


inline
bool simple_bounded_control::operator==(const simple_bounded_control &o) const
{
	BOOST_ASSERT(o.m_source);
	return operator==(*o.m_source);
}


inline
bool simple_bounded_control::operator==(const control &o) const
{
	BOOST_ASSERT(m_source);
	return source_comparator::compare(o, *m_source) == 0;
}


inline
bool simple_bounded_control::operator==(const SF &source) const
{
	return source_comparator::compare(*this, source) == 0;
}


inline
aggregated_control::~aggregated_control()
{
}

} /* namespace fancontrol */
#endif /* CONTROL_HPP_ */
