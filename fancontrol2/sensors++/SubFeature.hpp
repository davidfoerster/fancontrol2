/*
 * SubFeature.hpp
 *
 *  Created on: 16.12.2011
 *      Author: malte
 */

#pragma once
#ifndef SENSORS_SUBFEATURE_HPP_
#define SENSORS_SUBFEATURE_HPP_

#include "common.hpp"
#include "internal/ObjectWrapper.hpp"
#include "csensors.hpp"

namespace sensors {

class Feature;


class SubFeature: public ObjectWrapper<const sensors_subfeature, Feature>
{
public:
	typedef const sensors_subfeature T;

	typedef sensors_subfeature_type EType;

	static EType getTypeFromName(sensors_feature_type feature, const string_ref &name);

	struct Flags {
		enum value {
			readable,
			writable,
			compute_mapping,
			_size
		};
	};

	typedef Flags::value EFlags;

	struct WrappedTypeComparator {
		static int compare(const SubFeature &o1, const T *o2) {
			return o1.getNumber() - o2->number;
		}
	};

	SubFeature(T *subfeature, Feature *feature) : ObjectWrapper(subfeature, feature) {}

	bool operator==(const SubFeature &other) const;
	bool operator!=(const SubFeature &other) const { return !this->operator==(other); }

	const char *getName() const { return object->name; }

	int getNumber() const { return object->number; }

	EType getType() const { return object->type; }

	int getMapping() const { return object->mapping; }

	unsigned int getFlags() const { return object->flags; }

	bool testFlag(unsigned int flag) const { return getFlags() & flag; }

	bool testFlag(EFlags flag) const { return testFlag(1 << flag); }

	double getValue() const;

	void setValue(double) const;

private:

};

} /* namespace sensors */
#endif /* SENSORS_SUBFEATURE_HPP_ */
