/*
 * sensors.hpp
 *
 *  Created on: 16.12.2011
 *      Author: malte
 */

#pragma once
#ifndef SENSORS_SENSORS_HPP_
#define SENSORS_SENSORS_HPP_

#include "chip.hpp"
#include "internal/lock.hpp"
#include "exceptions.hpp"
#include "../util/static_allocator/static_allocator.hpp"

#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/functional/hash.hpp>
#include <unordered_map>

#include "csensors.hpp"
#include <cstdio>


namespace sensors {

	using ::boost::shared_ptr;


	class sensor_container {
	public:
		typedef sensors::chip chip_t;

		typedef ::std::unordered_map<
				chip_t::basic_type, shared_ptr<chip_t>,
				::boost::hash<chip_t::basic_type>, ::std::equal_to<chip_t::basic_type>,
				::util::static_allocator< ::std::pair<const chip_t::basic_type, shared_ptr<chip_t> >, 8 >
			> map_type;

		sensor_container(const char *config = default_config_path) throw (sensor_error, io_error, ::std::logic_error);

		map_type discover_all(const sensors_chip_name *match = 0);

		shared_ptr<chip_t> chip(const chip_t::basic_type &match, bool ignore_duplicate_matches = false) throw (sensor_error);

		shared_ptr<chip_t> operator[](const chip_t::basic_type &match) const;

		shared_ptr<chip_t> chip(const string_ref &name, bool ignore_duplicate_matches = false) throw (sensor_error);

		shared_ptr<chip_t> operator[](const string_ref &name) const;

		shared_ptr<chip_t> parse_name(const string_ref &name);

		const map_type &chips() const;

	private:
		static const shared_ptr<chip_t> &chip_internal(
				shared_ptr<chip_t> &chip,
				const chip_t::basic_type *match,
				bool ignore_duplicate_matches
			) throw (sensor_error);

		map_type m_chips;

		::boost::shared_ptr<lock> m_lock;

		typedef const map_type::key_type& (&get_key_t)(const map_type::value_type&);
	};



// implementations ========================================

	inline
	const sensor_container::map_type &sensor_container::chips() const
	{
		return m_chips;
	}

} /* namespace sensors */

#endif // SENSORS_SENSORS_HPP_
