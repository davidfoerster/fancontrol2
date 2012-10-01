/*
 * control.hpp
 *
 *  Created on: 22.12.2011
 *      Author: malte
 */

#pragma once
#ifndef CONTROL_HPP_
#define CONTROL_HPP_

#include "util/algorithm.hpp"

#include <boost/shared_ptr.hpp>
#include <vector>
#include <utility>
#include <stdexcept>
#include <functional>

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

	protected:
		control();

		virtual value_t rate_impl() const = 0;

	private:
		mutable value_t m_last_rate;
	};


	class bounded_control
		: public control
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


	class simple_bounded_control
		: public bounded_control
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

		struct source_comparator
			: ::std::binary_function<const control&, const SF&, bool>
		{
			bool operator()(const simple_bounded_control &o, const SF &source) const;

			bool operator()(const control &o, const SF &source) const;
		};

	protected:
		virtual value_t rate_impl() const;

	private:
		static const shared_ptr<const SF> &check_source_type(const shared_ptr<const SF> &m_gauge)
				throw(std::invalid_argument);

		shared_ptr<const SF> m_source;

		friend struct source_comparator;
	};


	class aggregated_control_base
		: public control
	{
	public:
		typedef shared_ptr<control> control_ptr_t;
		typedef control_ptr_t* iterator;
		typedef const control_ptr_t* const_iterator;

		virtual inline ~aggregated_control_base() { };

		virtual std::pair<iterator, iterator> sources() = 0;
		virtual std::pair<const_iterator, const_iterator> sources() const = 0;

	protected:
		virtual value_t rate_impl() const;
	};


	template <class Alloc = ::std::allocator<control> >
	class aggregated_control
		: public aggregated_control_base
	{
	public:
		typedef std::vector<shared_ptr<control>, Alloc> container_type;

		inline aggregated_control() { };

		virtual inline ~aggregated_control() { };

		inline explicit aggregated_control(const aggregated_control_base &other)
			: m_sources(other.sources().first, other.sources().second)
		{ }

		inline aggregated_control<Alloc> &operator=(const aggregated_control_base &other) {
			const std::pair<aggregated_control_base::const_iterator, aggregated_control_base::const_iterator>
				other_sources = other.sources();
			this->m_sources.assign(other_sources.first, other_sources.second);
			return *this;
		}

		virtual inline std::pair<iterator, iterator> sources() {
			return std::make_pair(m_sources.data(), m_sources.data() + m_sources.size());
		}

		virtual inline std::pair<const_iterator, const_iterator> sources() const {
			return std::make_pair(m_sources.data(), m_sources.data() + m_sources.size());
		}

		container_type m_sources;
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
		return m_last_rate = rate_impl();
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

} /* namespace fancontrol */
#endif /* CONTROL_HPP_ */
