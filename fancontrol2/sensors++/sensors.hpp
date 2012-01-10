/*
 * sensors.hpp
 *
 *  Created on: 16.12.2011
 *      Author: malte
 */

#pragma once
#ifndef SENSORS_HPP_
#define SENSORS_HPP_

#include "meta/GloballySharedResource.hpp"
#include <vector>
#include "csensors.hpp"

#include "Chip.hpp"

namespace sensors {


class Sensors: ObjectWrapper<const Sensors> {
public:
	Sensors();

	size_t discoverAll(const Chip::T *match = NULL);

	Chip *getChip(const char *name);

	Chip *parseChipName(const char *name);

	std::vector<Chip> chips;
};


} /* namespace sensors */
#endif /* SENSORS_HPP_ */
