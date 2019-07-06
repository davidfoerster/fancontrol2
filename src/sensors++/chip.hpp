/*
 * chip.hpp
 *
 *  Created on: 16.12.2011
 *      Author: malte
 */

#pragma once
#ifndef SENSORS_CHIP_HPP_
#define SENSORS_CHIP_HPP_

#include "internal/common.hpp"
#include "internal/conditional_shared_ptr.hpp"
#include "internal/lock.hpp"
#include "exceptions.hpp"

#include "util/memory.hpp"
#include <boost/functional/hash.hpp>
#include <unordered_map>
#include <bitset>

#include "csensors.hpp"
#include <boost/assert.hpp>


namespace sensors {

using util::shared_ptr;
using util::weak_ptr;

using util::io_error;

struct sensors_chip_name;
class feature;
class pwm;


class chip
	: virtual public util::enable_shared_from_this<chip>
	, public conditional_shared_ptr<sensors_chip_name>
{
public:
	typedef sensors_chip_name basic_type;
	typedef conditional_shared_ptr<basic_type> super;
	typedef sensors::feature feat_t;
	typedef sensors::pwm pwm_t;
	typedef std::pair<sensors_feature_type, int> feature_key_type;

	typedef std::unordered_map<
			feature_key_type, weak_ptr<feat_t>,
			boost::hash<feature_key_type>
		> feature_map_type;
	typedef std::unordered_map< unsigned int, weak_ptr<pwm_t> > pwm_map_type;

	struct Quirks {
		enum value {
			pwm_read_before_write,
			pwm2_alters_pwm1,
			_length
		};

		typedef std::bitset<_length> Set;
	};

	typedef Quirks::value quirks_enum;

	struct prefix_comparator
		: std::binary_function<const basic_type&, const string_ref&, bool>
	{
		bool operator()(const basic_type &chip, const string_ref &prefix) const;
	};

	chip(const basic_type *chip);

	chip(std::unique_ptr<basic_type> &chip);

	typename rebind_ptr<feature_map_type>::other discover_features();

	shared_ptr<feat_t> feature(const feature_map_type::key_type &);

	shared_ptr<feat_t> feature(sensors_feature_type type, int index);

	shared_ptr<feat_t> feature_consume_name(string_ref &name);

	typename rebind_ptr<pwm_map_type>::other discover_pwms();

	shared_ptr<pwm_t> pwm(unsigned number);

	const feature_map_type &features() const;

	const pwm_map_type &pwms() const;

	const Quirks::Set &quirks() const;

	const string_ref &prefix() const;

	const string_ref &path() const;

	bool operator==(const chip &other) const;

	static bool is_wildcard(const basic_type &chip);

protected:
	void init();

	void guess_quirks();

	void check_wildcard() const;

	feature_map_type m_features;

	pwm_map_type m_pwms;

	Quirks::Set m_quirks;

private:
	static void chip_deleter(sensors_chip_name *chip);

	string_ref m_prefix, m_path;

	lock::auto_lock m_autolock;
};

} /* namespace sensors */


template <typename Char, class Traits>
std::basic_ostream<Char, Traits> &operator<<(std::basic_ostream<Char, Traits>&, const sensors::chip&);



// implementations ========================================

namespace sensors {

inline
const chip::feature_map_type &chip::features() const
{
	return m_features;
}


inline
const chip::pwm_map_type &chip::pwms() const
{
	return m_pwms;
}


inline
const chip::Quirks::Set &chip::quirks() const
{
	return m_quirks;
}


inline
const string_ref &chip::path() const
{
	return m_path;
}


inline
const string_ref &chip::prefix() const
{
	return m_prefix;
}


inline
bool chip::operator==(const chip &other) const
{
	return this->get() == other.get();
}


inline bool chip::prefix_comparator::operator()(const basic_type &a, const string_ref &b) const
{
	return b == a.prefix;
}

} /* namespace sensors */


template <typename Char, class Traits>
std::basic_ostream<Char, Traits> &operator<<(std::basic_ostream<Char, Traits> &out, const sensors::chip &ch)
{
	if (!!ch) {
		out << ch.prefix() << ':' << ch->addr;
	} else {
		out << "<null>";
	}
	return out;
}


extern
template std::ostream &operator<<(std::ostream&, const sensors::chip&);

#endif /* SENSORS_CHIP_HPP_ */
