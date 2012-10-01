/*
 * chip.cpp
 *
 *  Created on: 16.12.2011
 *      Author: malte
 */

#include "chip.hpp"
#include "feature.hpp"
#include "pwm.hpp"
#include "../util/algorithm.hpp"

#include <boost/filesystem.hpp>
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include <boost/assert.hpp>
#include <cstring>


namespace sensors {

	using ::boost::weak_ptr;
	using ::boost::shared_ptr;
	using ::util::self_referenced;


	void chip::init() throw (sensor_error)
	{
		check_wildcard();
		guess_quirks();
	}


	typename rebind_shared_ptr<chip::feature_map_type>::other chip::discover_features()
	{
		typename rebind_shared_ptr<feature_map_type>::other map;
		if (!!*this) {
			const feat_t::basic_type *ft_basic;
			int nr = 0;
			while ((ft_basic = sensors_get_features(get(), &nr)) != 0) {
				const feature_map_type::key_type key(ft_basic->type, ft_basic->number);
				shared_ptr<feat_t> &dst = map[key];
				weak_ptr<feat_t> &src = m_features[key];
				if (!src.expired()) {
					dst = src.lock();
				} else {
					src = (dst = self_referenced<feat_t>::make(ft_basic, selfreference()));
				}
			}
		}
		return map;
	}


	typename rebind_shared_ptr<chip::pwm_map_type>::other chip::discover_pwms()
	{
		namespace fs = ::boost::filesystem;
		using ::std::string;
		using ::std::isdigit;

		const string_ref &prefix = pwm::Item::prefix();
		BOOST_ASSERT(!prefix.empty());

		typename rebind_shared_ptr<pwm_map_type>::other map;
		fs::path chip_path(this->path().begin(), this->path().end());
		for (fs::directory_iterator it(chip_path), end; it != end; ++it)
		{
			const fs::path feature_path(it->path().filename());
			const string &feature_name = feature_path.native();
			if (::util::has_prefix(feature_name, prefix))
			{
				string::const_iterator it = feature_name.begin() + prefix.length();
				if (it != feature_name.end() && *it != '0' && isdigit(*it)) {
					unsigned index = *it - '0';
					for (++it; it != feature_name.end() && isdigit(*it); ++it) {
						index = index * 10 + *it - '0';
					}
					if (it == feature_name.end()) {
						shared_ptr<pwm_t> p(pwm(index));
						if (p)
							map[index].swap(p);
					}
				}
			}
		}
		return map;
	}


	shared_ptr<pwm> chip::pwm(unsigned number)
	{
		if (number != 0 && !!*this) {
			weak_ptr<pwm_t> &p = m_pwms[number];
			if (!p.expired())
				return p.lock();

			shared_ptr<pwm_t> p_new(self_referenced<pwm_t>::make(number, selfreference()));
			if (p_new->exists()) {
				p = p_new;
				return p_new;
			}
		}
		return shared_ptr<pwm_t>();
	}


	shared_ptr<feature> chip::feature(const feature_map_type::key_type &key)
	{
		if (feat_t::Types::is_valid(key)) {
			weak_ptr<feat_t> &ft = m_features[key];

			if (!ft.expired())
				return ft.lock();

			if (!!*this) {
				const string_ref &prefix = feat_t::Types::name(key.first);
				BOOST_ASSERT(!prefix.empty());
				int nr = 0;
				const feat_t::basic_type *ft_basic;
				while (!!(ft_basic = sensors_get_features(get(), &nr))) {
					if (ft_basic->type == key.first) {
						BOOST_ASSERT(prefix.prefix_of(ft_basic->name));

						using ::std::isdigit;
						const char *number_str = ft_basic->name + prefix.length();
						BOOST_ASSERT(*number_str != '0' && isdigit(*number_str));

						int number = *number_str - '0';
						for (++number_str; isdigit(*number_str); ++number_str) {
							number = number * 10 + *number_str - '0';
						}
						BOOST_ASSERT(*number_str == '\0');
						if (number == key.second) {
							shared_ptr<feat_t> ft_new(self_referenced<feat_t>::make(
									ft_basic, string_ref(ft_basic->name, number_str), selfreference(), feat_t::key1()));
							ft = ft_new;
							return ft_new;
						}
					}
				}
			}
		}
		return shared_ptr<feat_t>();
	}


	shared_ptr<feature> chip::feature(feat_t::type_enum type, int index)
	{
		return feature(feature_map_type::key_type(type, index));
	}


	shared_ptr<feature> chip::feature_consume_name(string_ref &name)
	{
		feature_map_type::key_type which(feat_t::Types::from_name(name), 0);
		string_ref::const_iterator it = name.begin() + feat_t::Types::name(which.first).length();
		if (it < name.end() && *it != '0' && ::std::isdigit(*it)) {
				which.second = (*it++) - '0';
				while (it != name.end() && ::std::isdigit(*it)) {
					which.second = which.second * 10 + (*it++) - '0';
				}
				if (it == name.end() || *it == '_') {
					name = name.substr(it - name.begin() + 1);
					return feature(which);
				}
		}
		return shared_ptr<feat_t>();
	}


	void chip::guess_quirks()
	{
		if (!!*this) {
			if (prefix() == "w83667hg") {
				m_quirks.set(Quirks::pwm_read_before_write);
				m_quirks.set(Quirks::pwm2_alters_pwm1);
			}
		}
	}


	bool chip::is_wildcard(const basic_type &chip)
	{
		return chip.prefix == SENSORS_CHIP_NAME_PREFIX_ANY
				|| chip.addr == SENSORS_CHIP_NAME_ADDR_ANY
				|| chip.bus.nr == SENSORS_BUS_NR_ANY
				|| chip.bus.type == SENSORS_BUS_TYPE_ANY;
	}


	void chip::check_wildcard() const throw (sensor_error)
	{
		if (!!*this && is_wildcard(*get()))
			BOOST_THROW_EXCEPTION(sensor_error(sensor_error::misplaced_wildcard));
	}


	void chip::chip_deleter(sensors_chip_name *chip)
	{
		sensors_free_chip_name(chip);
		delete chip;
	}

} /* namespace sensors */


template ::std::ostream &operator<<(::std::ostream&, const ::sensors::chip&);

