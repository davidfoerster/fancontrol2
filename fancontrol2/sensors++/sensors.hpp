/*
 * sensors.hpp
 *
 *  Created on: 16.12.2011
 *      Author: malte
 */

#pragma once
#ifndef SENSORS_HPP_
#define SENSORS_HPP_

#include "chip.hpp"
#include "internal/lock.hpp"
#include "exceptions.hpp"

#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <map>

#include "csensors.hpp"
#include <cstdio>

namespace sensors {

using boost::shared_ptr;


class sensor_container {
public:
	typedef sensors::chip chip_t;

	typedef std::map<chip_t::basic_type, shared_ptr<chip_t>, chip_t::basic_chip_comparator> map_type;

	sensor_container(std::FILE *config = 0) throw (sensor_error, io_error, std::logic_error);

	map_type discover_all(const sensors_chip_name *match = 0);

	shared_ptr<chip_t> chip(const chip_t::basic_type &match, bool ignore_duplicate_matches = false) throw (sensor_error);

	shared_ptr<chip_t> operator[](const chip_t::basic_type &match) const;

	shared_ptr<chip_t> chip(const string_ref &name, bool ignore_duplicate_matches = false) throw (sensor_error);

	shared_ptr<chip_t> operator[](const string_ref &name) const;

	shared_ptr<chip_t> parse_name(const string_ref &name);

	const map_type &chips() const;

private:
	static shared_ptr<chip_t> &chip_internal(
			shared_ptr<chip_t> &chip,
			const chip_t::basic_type *chip_basic,
			bool ignore_duplicate_matches
		) throw (sensor_error);

	map_type m_chips;

	boost::shared_ptr<lock> m_lock;
};



// implementations ========================================

inline
const sensor_container::map_type &sensor_container::chips() const
{
	return m_chips;
}

} /* namespace sensors */

#endif /* SENSORS_HPP_ */
