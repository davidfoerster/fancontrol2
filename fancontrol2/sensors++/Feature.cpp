/*
 * MeasuringPoint.cpp
 *
 *  Created on: 14.12.2011
 *      Author: malte
 */

#include "Feature.hpp"
#include "Chip.hpp"
#include "meta/algorithm.hpp"
#include <algorithm>
#include <cassert>
#include <cstring>

namespace sensors {


bool Feature::operator==(const Feature &other) const
{
	const T
		*feat = this->object.get(),
		*other_feat = other.object.get();
	const bool b = ( feat == other_feat ||
		(	WrappedTypeComparator::compare(*this, other_feat) == 0 &&
			*this->parent == *other.parent &&
			true
		) );
	assert(!b || std::strcmp(feat->name, other_feat->name) == 0);
	return b;
}


SubFeature *Feature::getSubFeature(SubFeature::EType type)
{
	assert(get() && parent && parent->get());
	const SubFeature::T *sfeat_basic = sensors_get_subfeature(parent->get(), get(), type);
	if (sfeat_basic) {
		SubFeature *sf;
		std::vector<SubFeature>::iterator it =
				meta::find<SubFeature::WrappedTypeComparator>(subfeatures.begin(), subfeatures.end(), sfeat_basic);
		if (it == subfeatures.end()) {
			subfeatures.push_back(SubFeature(sfeat_basic, this));
			sf = &subfeatures.back();
		} else {
			sf = &(*it);
		}
		return sf;
	}
	return NULL;
}


SubFeature *Feature::getSubFeature(const string_ref &type_str)
{
	return getSubFeature(SubFeature::getTypeFromName(getType(), type_str));
}


size_t Feature::discoverAllSubfeatures()
{
	assert(get() && parent && parent->get());

	const SubFeature::T *sfeat_basic;
	const size_t oldsize = subfeatures.size();
	int nr = 0;

	while ((sfeat_basic = sensors_get_all_subfeatures(parent->get(), get(), &nr)) != NULL) {
		std::vector<SubFeature>::iterator end = subfeatures.begin(); std::advance(end, oldsize);
		if (meta::find<SubFeature::WrappedTypeComparator>(subfeatures.begin(), end, sfeat_basic) == end) {
			subfeatures.push_back(SubFeature(sfeat_basic, this));
		}
	}

	return subfeatures.size() - oldsize;
}


const Feature::TypeNamesType::ContainerType &Feature::TypeNames()
{
	static TypeNamesType *ans = new TypeNamesType;
	TypeNamesType::ContainerType &a = ans->container();
	if (a.empty()) {
		a.assign(a.capacity(), string_ref(NULL, 0));

		a[SENSORS_FEATURE_IN].cset("in");
		a[SENSORS_FEATURE_FAN].cset("fan");
		a[SENSORS_FEATURE_TEMP].cset("temp");
		a[SENSORS_FEATURE_POWER].cset("power");
		a[SENSORS_FEATURE_ENERGY].cset("energy");
		a[SENSORS_FEATURE_CURR].cset("curr");
		a[SENSORS_FEATURE_HUMIDITY].cset("humidity");
		a[SENSORS_FEATURE_MAX_MAIN].cset("max_main");
		a[SENSORS_FEATURE_VID].cset("vid");
		a[SENSORS_FEATURE_INTRUSION].cset("intrusion");
		a[SENSORS_FEATURE_MAX_OTHER].cset("max_other");
		a[SENSORS_FEATURE_BEEP_ENABLE].cset("beep_enable");
	}
	return a;
};


template <class InputIterator>
InputIterator findPrefix(InputIterator begin, const InputIterator &end, const string_ref &s)
{
	for (; begin != end; ++begin) {
		if (s.starts_with(*begin))
			return begin;
	}
	return end;
}


Feature::EType Feature::getTypeFromName(const string_ref &name)
{
	static const TypeNamesType::ContainerType &TypeNames = Feature::TypeNames();
	TypeNamesType::ContainerType::const_iterator type_name = findPrefix(TypeNames.begin(), TypeNames.end(), name);
	size_t type = type_name - TypeNames.begin();
	return (type < TypeNames.size()) ? static_cast<EType>(type) : SENSORS_FEATURE_UNKNOWN;
}

} /* namespace sensors */
