/*
 * feature.h
 *
 *  Created on: 14.12.2011
 *      Author: malte
 */

#pragma once
#ifndef SENSORS_FEATURE_HPP_
#define SENSORS_FEATURE_HPP_

#include "internal/common.hpp"
#include "internal/object_wrapper.hpp"
#include "csensors.hpp"
#include "chip.hpp"

#include "../util/exception.hpp"
#include <boost/functional/hash.hpp>
#include <boost/assert.hpp>
#include <array>
#include <unordered_map>
#include <memory>


namespace sensors {

	using std::shared_ptr;
	using std::weak_ptr;

	class subfeature;


	class feature
		: virtual public std::enable_shared_from_this<feature>
		, public object_wrapper_numbered<const sensors_feature, chip>
	{
	public:
		typedef object_wrapper_numbered<const sensors_feature, chip> super;

		typedef sensors_feature_type type_enum;

		typedef sensors::subfeature SF;

		typedef std::unordered_map<
				sensors_subfeature_type, weak_ptr<SF>,
				boost::hash<sensors_subfeature_type>
		> map_type;

		class key1 {
			key1() {}
			friend class chip;
		};

		struct Types {
			typedef std::array<string_ref, SENSORS_FEATURE_BEEP_ENABLE + 1> type_names_t;

			static type_names_t &make_names(type_names_t &a);

			static const type_names_t &names();

			static const string_ref &name(type_enum);

			static bool is_valid(type_enum type);

			static bool is_valid(type_enum type, const type_names_t &names);
			static bool is_valid(type_enum type, int index);

			static bool is_valid(const std::pair<type_enum, int>&);

			static bool check_name(const string_ref &name1, type_enum type2, int index2);

			static type_enum from_name(const string_ref &name);
		};

		feature(basic_type *feature, const shared_ptr<chip> &chip);

		feature(basic_type *feature, const string_ref &name, const shared_ptr<chip> &chip, key1);

		shared_ptr<SF> subfeature(sensors_subfeature_type type);
		shared_ptr<SF> operator[](sensors_subfeature_type type) const;

		shared_ptr<SF> subfeature(const string_ref &type);
		shared_ptr<SF> operator[](const string_ref &type) const;

		typename rebind_ptr<map_type>::other discover_all_subfeatures();

		const map_type &subfeatures() const;

		const string_ref &name() const;

		bool operator==(const feature &o) const;

	protected:
		string_ref m_name;

		map_type m_subfeatures;
	};

} /* namespace sensors */


template <typename Char, class Traits>
std::basic_ostream<Char, Traits> &operator<<(std::basic_ostream<Char, Traits>&, const sensors::feature&);



// implementations ========================================

namespace sensors {

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


	inline
	bool feature::operator==(const feature &o) const
	{
		return super::operator==(o);
	}

} /* namespace sensors */


template <typename Char, class Traits>
std::basic_ostream<Char, Traits> &operator<<(std::basic_ostream<Char, Traits> &out, const sensors::feature &feat)
{
	if (!!feat) {
		out << *UTIL_CHECK_POINTER(feat.parent()) << '/' << feat.name();
	} else {
		out << "<null>";
	}
	return out;
}

extern
template std::ostream &operator<<(std::ostream&, const sensors::feature&);


#endif // SENSORS_FEATURE_HPP_
