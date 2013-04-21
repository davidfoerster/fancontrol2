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
#include "util/static_allocator/static_vector.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/range/size.hpp>
#include <vector>
#include <utility>
#include <stdexcept>
#include <functional>
#include <algorithm>

#include <boost/assert.hpp>


#ifndef CONTROL_HPP_API
#	define CONTROL_HPP_API extern
#endif


namespace sensors {
	class subfeature;
}

namespace fancontrol {

	using boost::shared_ptr;
	class config;


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
			: std::binary_function<const control&, const SF&, bool>
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
		typedef std::pair<iterator, iterator> range_type;
		typedef std::pair<const_iterator, const_iterator> const_range_type;

		virtual ~aggregated_control_base();

		virtual range_type sources() = 0;
		virtual const_range_type sources() const = 0;

	protected:
		virtual value_t rate_impl() const;
	};


	template <std::size_t Size = 4, class Extent = std::allocator< shared_ptr<control> > >
	class aggregated_control
		: public aggregated_control_base
	{
	public:
		typedef aggregated_control<Size, Extent> self_t;
		typedef util::static_vector<shared_ptr<control>, Size, Extent> container_type;

		virtual ~aggregated_control();

		template <typename InputIterator>
		aggregated_control(InputIterator first, InputIterator last, typename container_type::size_type size_hint = 0);

		explicit aggregated_control(const aggregated_control_base &other);

		self_t &operator=(const aggregated_control_base &other);

		virtual range_type sources();

		virtual const_range_type sources() const;

	private:
		container_type m_sources;
	};


	CONTROL_HPP_API template class aggregated_control<>;



// implementation =============================================================

	inline
	control::control()
		: m_last_rate(0)
	{ }


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
		, m_lower_bound(0), m_upper_bound(1)
	{ }


	inline
	bounded_control::bounded_control(
			value_t lower_bound, value_t upper_bound,
			rate_conversion_fun_t rate_converter)
		: m_rate_converter(rate_converter)
		, m_lower_bound(lower_bound), m_upper_bound(upper_bound)
	{ }


	inline
	const shared_ptr<const simple_bounded_control::SF> &
	simple_bounded_control::source() const
	{
		BOOST_ASSERT(m_source);
		return m_source;
	}


	template <std::size_t S, class E>
	aggregated_control<S,E>::~aggregated_control()
	{ }


	template <std::size_t S, class E>
	template <typename InputIterator>
	aggregated_control<S,E>::aggregated_control(
			InputIterator first, InputIterator last,
			typename container_type::size_type size_hint
	) {
		m_sources.reserve(std::max(
				size_hint,
				static_cast<typename container_type::size_type>(container_type::allocator_type::initial_capacity)));
		m_sources.assign(first, last);
		BOOST_ASSERT(std::none_of(m_sources.begin(), m_sources.end(), std::logical_not< shared_ptr<control> >()));
	}


	template <std::size_t S, class E>
	inline
	aggregated_control<S,E>::aggregated_control(const aggregated_control_base &other)
	{
		const const_range_type other_sources(other.sources());
		m_sources.reserve(std::max(
				boost::size(other_sources),
				static_cast<typename container_type::difference_type>(container_type::allocator_type::initial_capacity)));
		m_sources.assign(other_sources.first, other_sources.second);
	}


	template <std::size_t S, class E>
	aggregated_control<S,E> &
	aggregated_control<S,E>::operator=(const aggregated_control_base &other)
	{
		aggregated_control_base::operator=(other);
		const const_range_type other_sources(other.sources());
		this->m_sources.assign(other_sources.first, other_sources.second);
		return *this;
	}


	template <std::size_t S, class E>
	aggregated_control_base::range_type aggregated_control<S,E>::sources()
	{
		return range_type(m_sources.data(), m_sources.data() + m_sources.size());
	}

	template <std::size_t S, class E>
	aggregated_control_base::const_range_type aggregated_control<S,E>::sources() const
	{
		return const_range_type(m_sources.data(), m_sources.data() + m_sources.size());
	}

} /* namespace fancontrol */

#undef CONTROL_HPP_API
#endif /* CONTROL_HPP_ */
