/*
 * feature.h
 *
 *  Created on: 14.12.2011
 *      Author: malte
 */

#pragma once
#ifndef SENSORS_FEATURE_H_
#define SENSORS_FEATURE_H_

#include "internal/common.hpp"
#include "internal/object_wrapper.hpp"

#include "meta/self_referenced.hpp"
#include <chromium/base/stack_container.h>
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/ref.hpp>

#include <map>

#include "csensors.hpp"
#include <boost/assert.hpp>

namespace sensors {

using boost::shared_ptr;
using boost::weak_ptr;

class chip;
class subfeature;


class feature
	: virtual public meta::self_referenced<feature>
	, public object_wrapper_numbered<const sensors_feature, chip>
{
public:
	typedef sensors_feature_type type_enum;

	typedef sensors::subfeature SF;

	typedef std::map<sensors_subfeature_type, weak_ptr<SF> > map_type;

	class key1 {
		key1() {}
		friend class chip;
	};

	struct Types {
		typedef chromium::StackVector<string_ref, SENSORS_FEATURE_BEEP_ENABLE + 1> type_names_t;

		static type_names_t &make_names(type_names_t &a);

		static const type_names_t::ContainerType &names();

		static const string_ref &name(type_enum);

		static bool is_valid(type_enum type);

		static bool is_valid(type_enum type, int index);

		static bool is_valid(const std::pair<type_enum, int>&);

		static bool check_name(const string_ref &name1, type_enum type2, int index2);

		static type_enum from_name(const string_ref &name);
	};

	struct NameComparator {
		static int compare(const basic_type *a, const basic_type *b);

		static int compare(const feature &a, const feature &b) {
			return compare(a, b.name());
		}
		static int compare(const feature &a, const string_ref &sa) {
			return a.name().compare(sa);
		}
	};

	template <class Tag>
	feature(basic_type *feature, const shared_ptr<chip> &chip, Tag tag);

	template <class Tag>
	feature(basic_type *feature, string_ref::size_type namelen, const shared_ptr<chip> &chip, key1, Tag);

	shared_ptr<SF> subfeature(sensors_subfeature_type type);
	shared_ptr<SF> operator[](sensors_subfeature_type type) const;

	shared_ptr<SF> subfeature(const string_ref &type);
	shared_ptr<SF> operator[](const string_ref &type) const;

	std::map<map_type::key_type, shared_ptr<SF> > discover_all_subfeatures();

	const map_type &subfeatures() const;

	const string_ref &name() const;

	bool operator==(const feature &o) const { return false; }

protected:
	string_ref m_name;

	map_type m_subfeatures;
};



// implementations ========================================

template <class Tag>
feature::feature(basic_type *feature, const shared_ptr<chip> &chip, Tag tag)
	: selfreference_type(tag)
	, object_wrapper_numbered(feature, chip)
	, m_name(feature ? feature->name : 0)
{
}


template <class Tag>
feature::feature(basic_type *feature, string_ref::size_type namelen, const shared_ptr<chip> &chip, key1, Tag tag)
	: selfreference_type(tag)
	, object_wrapper_numbered(feature, chip)
	, m_name(feature->name, namelen)
{
	BOOST_ASSERT(m_name.data() || namelen == 0);
}


inline
const feature::map_type &feature::subfeatures() const
{
	return m_subfeatures;
}


inline
const string_ref &feature::name() const
{
	return m_name;
}

} /* namespace sensors */
#endif /* SENSORS_FEATURE_H_ */
