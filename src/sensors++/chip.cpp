/*
 * chip.cpp
 *
 *  Created on: 16.12.2011
 *      Author: malte
 */

#include "chip.hpp"
#include "feature.hpp"
#include "pwm.hpp"
#include "util/algorithm.hpp"
#include "util/stringpiece/lexical_cast.hpp"

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/assert.hpp>
#include <cstring>


namespace sensors {

	using util::make_shared;


	chip::chip(const basic_type *chip)
	throw (sensor_error, io_error)
		: super(chip)
		, m_prefix(chip->prefix), m_path(chip->path)
		, m_autolock()
	{
		init();
	}


	chip::chip(std::unique_ptr<basic_type> &chip)
	throw (sensor_error, io_error)
		: super(chip)
		, m_prefix(chip->prefix), m_path(chip->path)
		, m_autolock()
	{
		init();
	}


	void chip::init() throw (sensor_error)
	{
		check_wildcard();
		guess_quirks();
	}


	typename rebind_ptr<chip::feature_map_type>::other chip::discover_features()
	{
		typename rebind_ptr<feature_map_type>::other map;
		if (!!*this) {
			const feat_t::basic_type *ft_basic;
			int nr = 0;
			while ((ft_basic = sensors_get_features(get(), &nr)) != nullptr) {
				const feature_map_type::key_type key(ft_basic->type, ft_basic->number);
				shared_ptr<feat_t> &dst = map[key];
				weak_ptr<feat_t> &src = m_features[key];
				if (!src.expired()) {
					dst = src.lock();
				} else {
					dst.reset(new feat_t(ft_basic, shared_from_this()));
					src = dst;
				}
			}
		}
		return map;
	}


	typename rebind_ptr<chip::pwm_map_type>::other chip::discover_pwms()
	{
		namespace fs = boost::filesystem;
		using std::string;

		const string_ref &prefix = pwm::Item::prefix();
		BOOST_ASSERT(!prefix.empty());

		typedef typename rebind_ptr<pwm_map_type>::other map_type;
		map_type map;
		fs::path chip_path(this->path().begin(), this->path().end());
		for (fs::directory_iterator it(chip_path), end; it != end; ++it)
		{
			const fs::path feature_path(it->path().filename());
			const string &feature_name = feature_path.native();
			if (util::has_prefix(feature_name, prefix))
			{
				const string_ref number_str(feature_name.data() + prefix.length());
				if (starts_with_nonzero_digit(number_str)) {
					util::streamstate streamstate;
					const unsigned index = util::lexical_cast<unsigned>(number_str, &streamstate);;
					if (streamstate.first & std::ios::eofbit) {
						shared_ptr<pwm_t> p(pwm(index));
						if (p) {
							BOOST_ASSERT(map.count(index) == 0);
							map[index] = std::move(p);
						}
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

			shared_ptr<pwm_t> p_new(util::make_shared<pwm_t>(
					number, shared_from_this()));
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

						const string_ref number_str(ft_basic->name + prefix.length());
						BOOST_ASSERT(starts_with_nonzero_digit(number_str));

						util::streamstate streamstate;
						const int number = util::lexical_cast<int>(number_str, &streamstate);
						BOOST_ASSERT(streamstate.first & std::ios::eofbit);

						if (number == key.second) {
							shared_ptr<feat_t> ft_new(util::make_shared<feat_t>(
								ft_basic, string_ref(ft_basic->name, number_str.end()),
								shared_from_this(), feat_t::key1()));
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
		bool success = false;
		feature_map_type::key_type which(feat_t::Types::from_name(name), 0);
		const string_ref number_str(name.substr(feat_t::Types::name(which.first).length()));
		if (starts_with_nonzero_digit(number_str)) {
			util::streamstate streamstate;
			which.second = util::lexical_cast<feature_map_type::key_type::second_type>(number_str, &streamstate);
			if (streamstate.first & std::ios::eofbit) {
				name = string_ref();
				success = true;
			} else {
				BOOST_ASSERT(streamstate.second >= 0);
				BOOST_ASSERT(static_cast<string_ref::size_type>(streamstate.second) < number_str.size());
				if (number_str[streamstate.second] == '_') {
					name = number_str.substr(static_cast<string_ref::size_type>(streamstate.second) + 1);
					success = true;
				}
			}
		}
		return success ? feature(which) : shared_ptr<feat_t>();
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


template std::ostream &operator<<(std::ostream&, const sensors::chip&);

