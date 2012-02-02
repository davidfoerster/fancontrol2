/*
 * feature.cpp
 *
 *  Created on: 14.12.2011
 *      Author: malte
 */

#include "feature.hpp"
#include "subfeature.hpp"
#include "chip.hpp"

#include <chromium/base/stack_container.h>
#include <boost/make_shared.hpp>

#include "meta/algorithm.hpp"
#include <algorithm>
#include <iterator>

#include <boost/assert.hpp>
#include <cstring>

namespace sensors {

using boost::shared_ptr;
using boost::weak_ptr;
using meta::self_referenced;


int feature::NameComparator::compare(const basic_type *a, const basic_type *b) {
	const char
			*sa = a ? a->name : "",
			*sb = b ? b->name : "";
	return std::strcmp(sa, sb);
}


shared_ptr<subfeature> feature::subfeature(subfeature::type_enum type)
{
	weak_ptr<SF> &sf = m_subfeatures[type];
	if (!sf.expired())
		return sf.lock();

	if (!!*this && parent() && !!*parent()) {
		const SF::basic_type *sf_basic = sensors_get_subfeature(parent()->get(), get(), type);
		if (sf_basic) {
			shared_ptr<SF> sf_new(self_referenced<SF>::make(sf_basic, selfreference()));
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


std::map<feature::map_type::key_type, shared_ptr<feature::SF> > feature::discover_all_subfeatures()
{
	std::map<map_type::key_type, shared_ptr<SF> > subfeatures;

	if (!!*this && parent() && !!*parent()) {
		int nr = 0;
		const SF::basic_type *sf_basic;
		while ((sf_basic = sensors_get_all_subfeatures(parent()->get(), get(), &nr)) != 0) {
			shared_ptr<SF> &dst = subfeatures[sf_basic->type];
			weak_ptr<SF> &src = this->m_subfeatures[sf_basic->type];
			if (src.expired()) {
				src = dst = self_referenced<SF>::make(sf_basic, selfreference());
			} else {
				dst = src.lock();
			}
		}
	}

	return subfeatures;
}


feature::Types::type_names_t &feature::Types::make_names(type_names_t &a)
{
	a.container().assign(a.container().capacity(), string_ref());

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

	return a;
};


const feature::Types::type_names_t::ContainerType &feature::Types::names()
{
	static type_names_t::ContainerType &ans = make_names(*new type_names_t()).container();
	return ans;
};


feature::type_enum feature::Types::from_name(const string_ref &name)
{
	if (!name.empty()) {
		const type_names_t::ContainerType &names = Types::names();
		size_t pos = find_prefix(names.begin(), names.end(), name) - names.begin();
		if (pos != names.size())
			return static_cast<type_enum>(pos);
	}
	return SENSORS_FEATURE_UNKNOWN;;
}


bool feature::Types::check_name(const string_ref &name1, type_enum type2, int index2)
{

	if (is_valid(type2, index2)) {
		const string_ref &name2 = names()[type2];
		if (name1.starts_with(name2)) {
			string_ref::const_iterator it(name1.begin() + name2.length());
			if (it != name1.end()) {
				int index1 = (*it++) - '0';
				if (index1 <= '9') {
					while (index1 < index2 && it != name1.end() && std::isdigit(*it))
						index1 = index1 * 10 + (*it++) - '0';
					return index1 == index2 && (it == name1.end() || *it == '_');
				}
			}
		}
	}
	return false;
}


const string_ref &feature::Types::name(type_enum type)
{
	const static string_ref &empty_string = *new string_ref();
	const type_names_t::ContainerType &names = Types::names();
	return (meta::in_range<ssize_t>(type, 0, names.size())) ?
			names[type] :
			empty_string;
}


bool feature::Types::is_valid(type_enum type)
{
	const type_names_t::ContainerType &names = Types::names();
	return meta::in_range<ssize_t>(type, 0, names.size()) && !names[type].empty();
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
