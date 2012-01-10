/*
 * Chip.cpp
 *
 *  Created on: 16.12.2011
 *      Author: malte
 */

#include "Chip.hpp"
#include "meta/algorithm.hpp"
#include <boost/filesystem.hpp>
#include <algorithm>
#include <iterator>
#include <cstring>
#include <cassert>

namespace sensors {


Chip::Chip(const T *chip, bool weak)
	: ObjectWrapper(chip)
	, pReferenceCounter(weak ? NULL : new size_t(1))
{
	guessQuirks();
}


Chip::Chip(const Chip &other)
	: ObjectWrapper(other)
	, features(other.features)
	, pwms(other.pwms)
	, mQuirks(other.mQuirks)
	, pReferenceCounter(other.pReferenceCounter)
{
	ref();
}


Chip &Chip::operator=(const Chip &other)
{
	object = other.object;
	features = other.features;
	pwms = other.pwms;
	mQuirks = other.mQuirks;
	pReferenceCounter = other.pReferenceCounter;
	ref();
	return *this;
}


void Chip::ref()
{
	if (pReferenceCounter)
		*pReferenceCounter += 1;
}


Chip::~Chip()
{
	if (pReferenceCounter) {
		if (*pReferenceCounter == 1) {
			delete pReferenceCounter;
			T *chip = const_cast<T*>(object.getUnprotectedPtr());
			sensors_free_chip_name(chip);
			delete chip;
		} else {
			assert(*pReferenceCounter != 0);
			*pReferenceCounter -= 1;
		}
	}
}


size_t Chip::discoverFeatures()
{
	assert(object.get());

	const Feature::T *feat;
	const size_t oldsize = features.size();
	int nr = 0;

	while ((feat = sensors_get_features(object.get(), &nr)) != NULL) {
		std::vector<Feature>::iterator begin(features.begin()), end(begin + oldsize);
		if (meta::find<Feature::WrappedTypeComparator>(begin, end, feat) == end) {
			features.push_back(Feature(feat, this));
		}
	}

	return features.size() - oldsize;
}


bool Chip::operator==(const T *other) const
{
	const T *chip = object.get();
	const bool b = ( chip == other ||
		(	chip->addr == other->addr &&
			chip->bus.nr == other->bus.nr &&
			true
		) );
	assert(!b || (chip->bus.type == other->bus.type && std::strcmp(chip->path, other->path) == 0));
	return b;
}


size_t Chip::discoverPwms()
{
	using namespace boost::filesystem;

	const size_t oldsize = pwms.size();
	std::string format = Pwm::Item::prefix().as_string();
	assert(format.find('%') == std::string::npos);
	format += "%u%n";

	for (directory_iterator it(getPath()), end; it != end; ++it) {
		const char *name = it->path().filename().c_str();
		unsigned nr;
		int count;
		if (std::sscanf(name, format.c_str(), &nr, &count) >= 1 && name[count] == 0 && name[Pwm::Item::prefix().length()] != '0') {
			std::vector<Pwm>::iterator end(pwms.begin()); std::advance(end, oldsize);
			if (meta::find< Pwm::PathComparator<const char*> >(pwms.begin(), end, it->path().c_str()) == end) {
				pwms.push_back(Pwm(it->path().string(), *this, nr));
			}
		}
	}

	return pwms.size() - oldsize;
}


Pwm *Chip::getPwm(int number)
{
	std::string pwmPath = Pwm::makeBasePath(*this, number);

	for (std::vector<Pwm>::iterator it(pwms.begin()); it != pwms.end(); ++it) {
		if (it->getChip()) {
			if (it->getNumber() == number && *it->getChip() == *this)
				return &(*it);
		} else {
			if (pwmPath == it->getPath())
				return &(*it);
		}
	}

	Pwm pwm(pwmPath, *this, number, true);
	if (pwm.exists()) {
		pwms.push_back(pwm);
		return &pwms.back();
	}

	return NULL;
}


Feature *Chip::getFeature(Feature::EType type, unsigned index, bool assumeSorted)
{
	if (type != SENSORS_FEATURE_UNKNOWN && index != 0 && !features.empty()) {
		assert(type < Feature::TypeNames().size());
		index -= 1;

		if (!assumeSorted)
			sortFeatures();

		std::vector<Feature>::iterator it = meta::find<Feature::TypeComparator>(features.begin(), features.end(), type);
		if (features.end() - it > index) {
			std::advance(it, index);
			if (it->getType() == type)
				return &(*it);
		}
	}
	return NULL;
}


Feature *Chip::getFeature(string_ref &name, bool assumeSorted)
{
	size_t p = std::min(name.find('_'), name.length());

	if (!assumeSorted)
		sortFeatures();

	std::vector<Feature>::iterator it = meta::find<Feature::NameComparator>(features.begin(), features.end(), name.substr(0, p));

	name = name.substr(p + 1);
	return (it != features.end()) ? &(*it) : NULL;
}


void Chip::sortFeatures()
{
	std::sort(features.begin(), features.end(), Feature::WrappedTypeComparator());
}


int Chip::PrefixComparator::compare(const Chip &chip, const char *prefix)
{
	return std::strcmp(chip.getPrefix(), prefix);
}


void Chip::guessQuirks()
{
	string_ref name(object->prefix);
	if (name == string_ref::make_const("w83667hg")) {
		mQuirks.set(Quirks::pwm_read_before_write);
	}
}

} /* namespace sensors */
