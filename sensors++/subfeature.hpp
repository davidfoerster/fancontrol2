/*
 * subfeature.hpp
 *
 *  Created on: 16.12.2011
 *      Author: malte
 */

#pragma once
#ifndef SENSORS_SUBFEATURE_HPP_
#define SENSORS_SUBFEATURE_HPP_

#include "internal/common.hpp"
#include "internal/object_wrapper.hpp"
#include "exceptions.hpp"

#include "meta/self_referenced.hpp"

#include "csensors.hpp"

namespace sensors {

using boost::shared_ptr;

class feature;


class subfeature
	: virtual public meta::self_referenced<subfeature>
	, public object_wrapper_numbered<const sensors_subfeature, feature>
{
public:
	typedef object_wrapper_numbered<const sensors_subfeature, feature> super;

	typedef sensors_subfeature_type type_enum;

	static type_enum type_from_name(sensors_feature_type feature, const string_ref &name);

	struct type_comparator {
		static bool equal(const basic_type *a, type_enum tb);
		static bool equal(const basic_type *a, const basic_type *b);
		static bool equal(const subfeature &a, type_enum tb);

		static bool less(const basic_type *a, type_enum tb);
		static bool less(const basic_type *a, const basic_type *b);
		static bool less(const subfeature &a, type_enum tb);
	};

	struct flags {
		enum value {
			readable,
			writable,
			compute_mapping,
			_size
		};
	};

	typedef flags::value flags_enum;

	template <class Tag>
	subfeature(basic_type *subfeature, const shared_ptr<feature> &feature, Tag);

	bool test_flag(unsigned int flag) const;

	bool test_flag(flags_enum flag) const;

	double value() const throw(sensor_error);

	void value(double) const throw(meta::null_pointer_exception, sensor_error);

	bool operator==(const super &o) const;

	bool operator<(const super &o) const;
};



// implementations ========================================


template <class Tag>
subfeature::subfeature(basic_type *subfeature, const shared_ptr<feature> &feature, Tag tag)
	: selfreference_type(tag)
	, object_wrapper_numbered(subfeature, feature)
{
}


inline
bool subfeature::test_flag(flags_enum flag) const
{
	return test_flag(1U << flag);
}


inline
bool subfeature::type_comparator::equal(const basic_type *a, type_enum tb)
{
	return a && a->type == tb;
}

inline
bool subfeature::type_comparator::equal(const basic_type *a, const basic_type *b)
{
	return (a && b) ?
			a->type == b->type :
			a == b;
}

inline
bool subfeature::type_comparator::equal(const subfeature &a, type_enum tb)
{
	return equal(a.get(), tb);
}


inline
bool subfeature::type_comparator::less(const basic_type *a, type_enum tb)
{
	return !a || a->type < tb;
}

inline
bool subfeature::type_comparator::less(const basic_type *a, const basic_type *b)
{
	return (a && b) ?
			a->type < b->type :
			a < b;
}

inline
bool subfeature::type_comparator::less(const subfeature &a, type_enum tb)
{
	return less(a.get(), tb);
}

} /* namespace sensors */
#endif /* SENSORS_SUBFEATURE_HPP_ */
