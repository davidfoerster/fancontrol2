/*
 * Feature.h
 *
 *  Created on: 14.12.2011
 *      Author: malte
 */

#pragma once
#ifndef SENSORS_FEATURE_H_
#define SENSORS_FEATURE_H_

#include "common.hpp"
#include "internal/ObjectWrapper.hpp"
#include <chromium/base/stack_container.h>
#include <vector>
#include "csensors.hpp"

namespace sensors {
	class Chip;
}

#include "SubFeature.hpp"

namespace sensors {


class Feature: public ObjectWrapper<const sensors_feature, Chip>
{
public:
	typedef const sensors_feature T;

	typedef sensors_feature_type EType;

	typedef chromium::StackVector<string_ref, SENSORS_FEATURE_BEEP_ENABLE + 1> TypeNamesType;

	static const TypeNamesType::ContainerType &TypeNames();

	static EType getTypeFromName(const string_ref &name);

	struct WrappedTypeComparator {
		static int compare(const Feature &o1, const T *o2) {
			return o1.getNumber() - o2->number;
		}

		static int compare(const Feature &o1, const Feature &o2) {
			return compare(o1, o2.get());
		}

		bool operator()(const Feature &o1, const Feature &o2) const {
			return (compare(o1, o2) < 0);
		}
	};

	struct TypeComparator {
		static int compare(const Feature &o, EType type) {
			return o.getType() - type;
		}
	};

	struct NameComparator {
		static int compare(const Feature &o1, const string_ref &s2) {
			return string_ref(o1.getName()).compare(s2);
		}
	};

	Feature(T *feature, Chip *chip) : ObjectWrapper(feature, chip) {}

	SubFeature *getSubFeature(SubFeature::EType type);

	SubFeature *getSubFeature(const string_ref &type);

	size_t discoverAllSubfeatures();

	bool operator==(const Feature &other) const;
	bool operator!=(const Feature &other) const { return !this->operator==(other); }

	EType getType() const { return object->type; }

	int getNumber() const { return object->number; }

	const char *getName() const { return object->name; }

	std::vector<SubFeature> subfeatures;

private:

};

} /* namespace sensors */
#endif /* SENSORS_FEATURE_H_ */
