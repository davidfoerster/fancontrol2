/*
 * sensors.cpp
 *
 *  Created on: 16.12.2011
 *      Author: malte
 */

#include "sensors.hpp"
#include "Lock.hpp"
#include "SensorError.hpp"
#include "meta/algorithm.hpp"
#include <algorithm>
#include <iterator>

namespace sensors {


Sensors::Sensors()
	: ObjectWrapper(0)
{
	Lock::instance().init();
}


size_t Sensors::discoverAll(const Chip::T *match)
{
	const Chip::T *chip;
	const size_t oldsize = chips.size();
	int nr = 0;

	while ((chip = sensors_get_detected_chips(match, &nr)) != NULL) {
		std::vector<Chip>::iterator begin(chips.begin()), end(begin + oldsize);
		if (std::find(begin, end, chip) == end) {
			chips.push_back(Chip::makeInstance(chip));
		}
	}

	return chips.size() - oldsize;
}


Chip *Sensors::getChip(const char *name)
{
	// do we know that chip already?
	std::vector<Chip>::iterator it_chip = meta::find<Chip::PrefixComparator>(chips.begin(), chips.end(), name);
	if (it_chip != chips.end()) {
		if (meta::find<Chip::PrefixComparator>(it_chip + 1, chips.end(), name) != chips.end())
			throw SensorError(SensorError::misplaced_wildcard);
	}

	{ // if necessary, detect the chip; test its uniqueness
		const Chip::T match = { const_cast<char*>(name), { SENSORS_BUS_TYPE_ANY, SENSORS_BUS_NR_ANY }, SENSORS_CHIP_NAME_ADDR_ANY, NULL };
		int nr = 0;
		const Chip::T *basic_chip = sensors_get_detected_chips(&match, &nr);
		assert(basic_chip != &match);
		assert(!basic_chip || basic_chip->prefix != name);

		if (it_chip != chips.end()) {
			basic_chip = it_chip->get();  // intentiously overwrite the first of the search results (we just need to count them)
		} else if (basic_chip) {
			chips.push_back(Chip::makeInstance(basic_chip));
			it_chip = chips.end() - 1;
		}

		if (basic_chip && sensors_get_detected_chips(&match, &nr) != NULL)
			throw SensorError(SensorError::misplaced_wildcard);
	}

	return &(*it_chip);
}


Chip *Sensors::parseChipName(const char *name)
{
	int errnum;
	Chip::T *chip = new Chip::T;
	if ((errnum = sensors_parse_chip_name(name, chip)) != SensorError::no_error) {
		delete chip;
		if (SensorError::toEnum(errnum) != SensorError::unparsable_chip_name)
			throw SensorError(errnum);
		return NULL;
	}

	std::vector<Chip>::iterator p = std::find(chips.begin(), chips.end(), chip);
	if (p != chips.end()) {
		sensors_free_chip_name(chip);
		delete chip;
		return &(*p);
	}

	chips.push_back(Chip::makeInstance(chip));
	return &chips.back();
}

} /* namespace sensors */
