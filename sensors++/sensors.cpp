/*
 * sensors.cpp
 *
 *  Created on: 16.12.2011
 *      Author: malte
 */

#include "sensors.hpp"

#include "../util/algorithm.hpp"
#include "../util/self_referenced.hpp"
#include <boost/make_shared.hpp>
#include <boost/ref.hpp>
#include <boost/bind.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/assert.hpp>
#include <algorithm>
#include <iterator>


namespace sensors {

	using ::boost::weak_ptr;
	using ::boost::shared_ptr;
	using ::util::self_referenced;
	using ::util::io_error;


	sensor_container::sensor_container(const char *config) throw (sensor_error, io_error, ::std::logic_error)
		: m_chips(map_type::allocator_type::initial_capacity)
		, m_lock(lock::instance(false))
	{
		m_lock->init(config);
	}


	sensor_container::map_type sensor_container::discover_all(const sensors_chip_name *match)
	{
		map_type map;

		const chip_t::basic_type *chip_basic; int nr = 0;
		while (!!(chip_basic = sensors_get_detected_chips(match, &nr))) {
			shared_ptr<chip_t> &chip = m_chips[*chip_basic];
			if (!chip) {
				chip = self_referenced<chip_t>::make(chip_basic);
			}
			map[*chip_basic] = chip;
		}

		return map;
	}


	shared_ptr<chip> sensor_container::operator[](const chip_t::basic_type &match) const
	{
		map_type::const_iterator it(m_chips.find(match));
		return (it != m_chips.end()) ? it->second : shared_ptr<chip_t>();
	}


	const shared_ptr<chip> &sensor_container::chip_internal(
			shared_ptr<chip_t> &chip,
			const chip_t::basic_type *match,
			bool ignore_duplicate_matches
	) throw (sensor_error)
	{
		if (!chip || !ignore_duplicate_matches) {
			int nr = 0;
			const chip_t::basic_type *const chip_basic = sensors_get_detected_chips(match, &nr);
			if (chip_basic) {
				if (!ignore_duplicate_matches && !!sensors_get_detected_chips(match, &nr))
					BOOST_THROW_EXCEPTION(sensor_error(sensor_error::misplaced_wildcard));

				chip = self_referenced<chip_t>::make(chip_basic);
			}
		}
		return chip;
	}


	shared_ptr<chip> sensor_container::chip(
			const chip_t::basic_type &match,
			bool ignore_duplicate_matches
	) throw (sensor_error)
	{
		return chip_internal(m_chips[match], &match, ignore_duplicate_matches);
	}


	shared_ptr<chip> sensor_container::operator[](const string_ref &name) const
	{
		using ::boost::bind;
		const map_type::const_iterator it = ::boost::find_if(m_chips,
				bind(chip_t::prefix_comparator(), bind(static_cast<get_key_t>(::std::get<0>), _1), ::boost::ref(name)));
		return (it != m_chips.end()) ? it->second : shared_ptr<chip_t>();
	}


	shared_ptr<chip> sensor_container::chip(
			const string_ref &name,
			bool ignore_duplicate_matches
	) throw (sensor_error)
	{
		// do we know that chip already?
		using ::boost::bind;
		const map_type::iterator it_chip = ::boost::find_if(m_chips,
				bind(chip_t::prefix_comparator(), bind(static_cast<get_key_t>(::std::get<0>), _1), ::boost::ref(name)));

		if (it_chip != m_chips.end()) {
			BOOST_ASSERT(it_chip->second);
			BOOST_ASSERT(it_chip->first == *it_chip->second->get());

			if (ignore_duplicate_matches)
				return it_chip->second;
		}

		// if necessary, detect the chip; test its uniqueness
		int nr = 0;
		BOOST_ASSERT(!name.begin() || !*name.end());
		const chip_t::basic_type match = {
				const_cast<char*>(name.c_str()),
				{ SENSORS_BUS_TYPE_ANY, SENSORS_BUS_NR_ANY },
				SENSORS_CHIP_NAME_ADDR_ANY, 0
			},
			*basic_chip = sensors_get_detected_chips(&match, &nr);

		if (!basic_chip)
			return shared_ptr<chip_t>();

		BOOST_ASSERT(basic_chip != &match);
		BOOST_ASSERT(!basic_chip || basic_chip->prefix != name.begin());

		shared_ptr<chip_t> &chip = (it_chip != m_chips.end()) ?
			it_chip->second :
			(m_chips[*basic_chip] = self_referenced<chip_t>::make(basic_chip));

		if (!ignore_duplicate_matches && !!sensors_get_detected_chips(&match, &nr))
			BOOST_THROW_EXCEPTION(sensor_error(sensor_error::misplaced_wildcard));

		return chip;
	}


	shared_ptr<chip> sensor_container::parse_name(const string_ref &name)
	{
		::std::unique_ptr<chip_t::basic_type> chip_basic(new chip_t::basic_type);
		int errnum = sensors_parse_chip_name(name.c_str(), chip_basic.get());
		if (errnum == sensor_error::no_error) {
			shared_ptr<chip_t> &chip = m_chips[*chip_basic];
			if (!chip) {
				chip = self_referenced<chip_t>::make(::boost::ref(chip_basic));
			}
			return chip;
		} else {
			if (sensor_error::to_enum(errnum) != sensor_error::unparsable_chip_name)
				BOOST_THROW_EXCEPTION(sensor_error(errnum));
			return shared_ptr<chip_t>();
		}
	}

} /* namespace sensors */
