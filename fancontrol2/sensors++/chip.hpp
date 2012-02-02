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

#include "meta/self_referenced.hpp"
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <map>
#include <bitset>

#include "csensors.hpp"
#include <boost/assert.hpp>

namespace sensors {

using boost::shared_ptr;
using boost::weak_ptr;

struct sensors_chip_name;
class feature;
class pwm;


class chip
	: virtual public meta::self_referenced<chip>
	, public conditional_shared_ptr<sensors_chip_name>
{
public:
	typedef sensors_chip_name basic_type;
	typedef conditional_shared_ptr<basic_type> super;
	typedef sensors::feature feat_t;
	typedef sensors::pwm pwm_t;

	typedef std::map< std::pair<sensors_feature_type, int>, weak_ptr<feat_t> > feature_map_type;
	typedef std::map< unsigned int, weak_ptr<pwm_t> > pwm_map_type;

	struct Quirks {
		enum value {
			pwm_read_before_write,
			_length
		};

		typedef std::bitset<_length> Set;
	};

	typedef Quirks::value quirks_enum;

	struct mixed_comparator {
		static int compare(const basic_type *a, const basic_type *b);

		static int compare(const basic_type &a, const basic_type &b);

		static int compare(const chip &a, const basic_type *b);

	private:
		static int compare_internal(const basic_type &a, const basic_type &b);
	};

	struct prefix_comparator {
		static int compare(const basic_type &chip, const char *prefix);
		static int compare(const basic_type *chip, const char *prefix);

		static int compare(const chip &chip, const char *prefix);
		static int compare(const std::pair<basic_type, shared_ptr<chip> > &o, const char *prefix);

		static int compare(const chip &chip, const string_ref &prefix);
		static int compare(const std::pair<basic_type, shared_ptr<chip> > &o, const string_ref &prefix);
	};

	struct basic_chip_comparator {
		static int compare(const basic_type *a, const basic_type *b);
		bool operator()(const basic_type *a, const basic_type *b) const { return compare(a, b) < 0; }

		static int compare(const basic_type &a, const basic_type &b);
		bool operator()(const basic_type &a, const basic_type &b) const { return compare(a, b) < 0; }

		static int compare(const chip &a, const basic_type *b);
		bool operator()(const chip &a, const basic_type *b) const { return compare(a, b) < 0; }

	private:
		static int compare_internal(const basic_type &a, const basic_type &b);

		friend class mixed_comparator;
	};

	template <class Tag>
	chip(const basic_type *chip, Tag) throw (sensor_error, io_error);

	template <class Tag>
	chip(std::auto_ptr<basic_type> &chip, Tag) throw (sensor_error, io_error);

	std::map<feature_map_type::key_type, shared_ptr<feat_t> > discover_features();

	shared_ptr<feat_t> feature(const feature_map_type::key_type &);

	shared_ptr<feat_t> feature(sensors_feature_type type, int index);

	shared_ptr<feat_t> feature_consume_name(string_ref &name);

	std::map<pwm_map_type::key_type, shared_ptr<pwm_t> > discover_pwms();

	shared_ptr<pwm_t> pwm(unsigned number);

	const feature_map_type &features() const;

	const pwm_map_type &pwms() const;

	const Quirks::Set &quirks() const;

	const string_ref &prefix() const;

	const string_ref &path() const;

	bool operator==(const basic_type *other) const;

	bool operator==(const chip &other) const;

	static bool is_wildcard(const basic_type &chip);

protected:
	void init() throw (sensor_error);

	void guess_quirks();

	void check_wildcard() const throw (sensor_error);

	feature_map_type m_features;

	pwm_map_type m_pwms;

	Quirks::Set m_quirks;

private:
	static void chip_deleter(sensors_chip_name *chip);

	string_ref m_prefix, m_path;

	lock::auto_lock m_autolock;
};



// implementations ========================================

template <class Tag>
chip::chip(const basic_type *chip, Tag tag)
throw (sensor_error, io_error)
	: selfreference_type(tag)
	, super(chip)
	, m_prefix(chip->prefix), m_path(chip->path)
	, m_autolock()
{
	init();
}


template <class Tag>
chip::chip(std::auto_ptr<basic_type> &chip, Tag tag)
throw (sensor_error, io_error)
	: selfreference_type(tag)
	, super(chip)
	, m_prefix(chip->prefix), m_path(chip->path)
	, m_autolock()
{
	init();
}


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
	return operator==(other.get());
}


inline
int chip::prefix_comparator::compare(const basic_type *chip, const char *prefix)
{
	return chip ? compare(*chip, prefix) : (prefix && *prefix) ? 1 : 0;
}


inline
int chip::prefix_comparator::compare(const chip &chip, const char *prefix)
{
	return compare(chip.get(), prefix);
}

inline
int chip::prefix_comparator::compare(const std::pair<basic_type, shared_ptr<chip> > &o, const char *prefix)
{
	BOOST_ASSERT(o.second && *o.second == &o.first);
	return compare(o.first, prefix);
}


inline
int chip::prefix_comparator::compare(const chip& chip, const string_ref &prefix)
{
	return chip.prefix().compare(prefix);
}

inline
int chip::prefix_comparator::compare(const std::pair<basic_type, shared_ptr<chip> > &o, const string_ref &prefix)
{
	BOOST_ASSERT(o.second && *o.second == &o.first);
	return compare(o.first, prefix.data());
}


inline
int chip::basic_chip_comparator::compare(const basic_type &a, const basic_type &b)
{
	return (&a == &b) ? 0 : compare_internal(a, b);
}


inline
int chip::basic_chip_comparator::compare(const chip &a, const basic_type *b)
{
	return compare(a.get(), b);
}


inline
int chip::mixed_comparator::compare(const basic_type &a, const basic_type &b)
{
	return (&a == &b) ? 0 : compare_internal(a, b);
}


inline
int chip::mixed_comparator::compare(const chip &a, const basic_type *b)
{
	return compare(a.get(), b);
}

} /* namespace sensors */
#endif /* SENSORS_CHIP_HPP_ */
