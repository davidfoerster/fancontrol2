/*
 * Chip.hpp
 *
 *  Created on: 16.12.2011
 *      Author: malte
 */

#pragma once
#ifndef SENSORS_CHIP_HPP_
#define SENSORS_CHIP_HPP_

#include "internal/ObjectWrapper.hpp"
#include <vector>
#include <bitset>
#include "csensors.hpp"

#include "Feature.hpp"
#include "Pwm.hpp"

namespace sensors {


class Chip: public ObjectWrapper<const sensors_chip_name>
{
public:
	typedef sensors_chip_name T;

	struct Quirks {
		enum value {
			pwm_read_before_write,
			_length
		};

		typedef std::bitset<_length> Set;
	};

	typedef Quirks::value EQuirks;

	struct PrefixComparator {
		static int compare(const Chip& chip, const char *prefix);
	};

	~Chip();

	static Chip makeInstance(const T *chip) { return Chip(chip, true); }
	static Chip makeInstance(T *chip) { return Chip(chip, false); }

	Chip(const Chip &other);

	Chip &operator=(const Chip &other);

	size_t discoverFeatures();

	Feature *getFeature(Feature::EType type, unsigned index = 1, bool assumeSorted = false);

	Feature *getFeature(string_ref &name, bool assumeSorted = false);

	size_t discoverPwms();

	Pwm *getPwm(int number);

	const char *getPrefix() const { return object->prefix; }

	const sensors_bus_id &getBusId() const { return object->bus; }

	int getAddress() const { return object->addr; }

	const char *getPath() const { return object->path; }

	const Quirks::Set &getQuirks() const { return mQuirks; }

	bool operator==(const T *other) const;
	bool operator!=(const T *other) const { return !operator==(other); }

	bool operator==(const Chip &other) const { return operator==(other.get()); }
	bool operator!=(const Chip &other) const { return !operator==(other); }

	void sortFeatures();

	std::vector<Feature> features;

	std::vector<Pwm> pwms;

protected:
	Chip(const T *chip, bool weak);

	void ref();

	void guessQuirks();

	Quirks::Set mQuirks;

private:
	size_t *pReferenceCounter;
};

} /* namespace sensors */
#endif /* SENSORS_CHIP_HPP_ */
