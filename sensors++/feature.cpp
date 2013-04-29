/*
 * feature.cpp
 *
 *  Created on: 14.12.2011
 *      Author: malte
 */

#define SENSORS_FEATURE_H_BUILD
#include "feature.hpp"
#include "subfeature.hpp"
#include "chip.hpp"

#include "../util/algorithm.hpp"
#include "../util/stringpiece/lexical_cast.hpp"
#include <boost/range/algorithm/find_if.hpp>
#include <boost/assert.hpp>
#include <algorithm>
#include <iterator>
#include <functional>


namespace sensors {

	using std::shared_ptr;
	using std::weak_ptr;
	using std::make_shared;


	feature::feature(basic_type *feature, const shared_ptr<chip> &chip)
		: object_wrapper_numbered(feature, chip)
		, m_name(feature ? feature->name : 0)
	{
	}


	feature::feature(basic_type *feature, const string_ref &name, const shared_ptr<chip> &chip, key1)
		: object_wrapper_numbered(feature, chip)
		, m_name(name)
	{
		BOOST_ASSERT(!feature || name == feature->name);
	}


	shared_ptr<subfeature> feature::subfeature(subfeature::type_enum type)
	{
		weak_ptr<SF> &sf = m_subfeatures[type];
		if (!sf.expired())
			return sf.lock();

		if (!!*this && parent() && !!*parent()) {
			const SF::basic_type *sf_basic = sensors_get_subfeature(parent()->get(), get(), type);
			if (sf_basic) {
				shared_ptr<SF> sf_new(make_shared<SF>(sf_basic, shared_from_this()));
				sf = sf_new;
				return sf_new;
			}
		}

		return shared_ptr<feature::SF>();
	}


	shared_ptr<subfeature> feature::subfeature(const string_ref &type_str)
	{
		return !!*this ? subfeature(SF::type_from_name(m_object->type, type_str)) : shared_ptr<feature::SF>();
	}


	shared_ptr<subfeature> feature::operator[](SF::type_enum type) const
	{
		map_type::const_iterator it(m_subfeatures.find(type));
		return (it != m_subfeatures.end()) ? it->second.lock() : shared_ptr<SF>();
	}


	shared_ptr<subfeature> feature::operator[](const string_ref &type_str) const
	{
		if (!!*this) {
			SF::type_enum type = SF::type_from_name(m_object->type, type_str);
			if (type != SENSORS_SUBFEATURE_UNKNOWN) {
				map_type::const_iterator it(m_subfeatures.find(type));
				if (it != m_subfeatures.end()) {
					return it->second.lock();
				}
			}
		}
		return shared_ptr<SF>();
	}


	typename rebind_ptr<feature::map_type>::other feature::discover_all_subfeatures()
	{
		typename rebind_ptr<map_type>::other subfeatures;

		if (!!*this && parent() && !!*parent()) {
			int nr = 0;
			const SF::basic_type *sf_basic;
			while ((sf_basic = sensors_get_all_subfeatures(parent()->get(), get(), &nr)) != 0) {
				shared_ptr<SF> &dst = subfeatures[sf_basic->type];
				weak_ptr<SF> &src = this->m_subfeatures[sf_basic->type];
				if (src.expired()) {
					dst.reset(new SF(sf_basic, shared_from_this()));
					src = dst;
				} else {
					dst = src.lock();
				}
			}
		}

		return subfeatures;
	}


	feature::Types::type_names_t &feature::Types::make_names(type_names_t &a)
	{
		a[SENSORS_FEATURE_IN] = "in";
		a[SENSORS_FEATURE_FAN] = "fan";
		a[SENSORS_FEATURE_TEMP] = "temp";
		a[SENSORS_FEATURE_POWER] = "power";
		a[SENSORS_FEATURE_ENERGY] = "energy";
		a[SENSORS_FEATURE_CURR] = "curr";
		a[SENSORS_FEATURE_HUMIDITY] = "humidity";
		a[SENSORS_FEATURE_MAX_MAIN] = "max_main";
		a[SENSORS_FEATURE_VID] = "vid";
		a[SENSORS_FEATURE_INTRUSION] = "intrusion";
		a[SENSORS_FEATURE_MAX_OTHER] = "max_other";
		a[SENSORS_FEATURE_BEEP_ENABLE] = "beep_enable";

		return a;
	};


	const feature::Types::type_names_t &feature::Types::names()
	{
		static type_names_t &ans = make_names(*new type_names_t());
		return ans;
	};


	feature::type_enum feature::Types::from_name(const string_ref &name)
	{
		using std::bind;
		using std::cref;
		using namespace std::placeholders;
		using boost::find_if;
		using util::has_prefix;
		if (!name.empty()) {
			const type_names_t &names = Types::names();
			const type_names_t::const_iterator pos =
					find_if(names, bind(has_prefix<string_ref>, cref(name), _1));
			if (pos != names.end())
				return static_cast<type_enum>(pos - names.begin());
		}
		return SENSORS_FEATURE_UNKNOWN;;
	}


	bool feature::Types::check_name(const string_ref &name1, type_enum type2, int index2)
	{

		if (is_valid(type2, index2)) {
			const string_ref &name2 = names()[type2];
			if (util::has_prefix(name1, name2)) {
				const string_ref number_str(name1.substr(name2.size()));
				if (starts_with_nonzero_digit(number_str)) {
					util::streamstate streamstate;
					const int index1 = util::lexical_cast<int>(number_str, &streamstate);
					return index1 == index2 &&
						( (streamstate.first & std::ios::eofbit) ||
						  number_str[streamstate.second] == '_' );
				}
			}
		}
		return false;
	}


	const string_ref &feature::Types::name(type_enum type)
	{
		const type_names_t &names = Types::names();
		return is_valid(type, names) ? names[type] : string_ref::empty_string();
	}


	bool feature::Types::is_valid(type_enum type)
	{
		const type_names_t &names = Types::names();
		return is_valid(type, names) && !names[type].empty();
	}


	bool feature::Types::is_valid(type_enum type, const type_names_t &names)
	{
		return type >= 0 && type < names.size();
	}


	bool feature::Types::is_valid(type_enum type, int index)
	{
		return index != 0 && is_valid(type);
	}


	bool feature::Types::is_valid(const std::pair<type_enum, int> &which)
	{
		return is_valid(which.first, which.second);
	}

} /* namespace sensors */
