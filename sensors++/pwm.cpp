/*
 * Pwm.cpp
 *
 *  Created on: 24.12.2011
 *      Author: malte
 */

#include "pwm.hpp"
#include "chip.hpp"

#include <boost/filesystem.hpp>
#include <sstream>
#include <fstream>

#include <algorithm>
#include <limits>
#include <stdexcept>

#include <cstring>
#include <boost/assert.hpp>
#include <unistd.h>

namespace sensors {

using boost::shared_ptr;
using meta::io_error;


pwm::Item::names_type &pwm::Item::make_names(names_type &names)
{
	names_type::ContainerType &a = names.container();
	BOOST_ASSERT(a.capacity() == _length);
	a.clear();

	a.push_back(STRING_REF(""));
	a.push_back(STRING_REF("enable"));
	a.push_back(STRING_REF("mode"));
	a.push_back(STRING_REF("freq"));
	a.push_back(STRING_REF("start_output"));
	a.push_back(STRING_REF("stop_output"));
	a.push_back(STRING_REF("target"));
	a.push_back(STRING_REF("tolerance"));
	a.push_back(STRING_REF("auto_channels_temp"));

	BOOST_ASSERT(a.size() == a.capacity());
	return names;
}


const pwm::Item::names_type::ContainerType &pwm::Item::names()
{
	static names_type::ContainerType &a = make_names(*new names_type()).container();
	return a;
}


string_ref pwm::Item::prefix()
{
	return string_ref::make_const("pwm");
}


pwm::pwm(const std::string &path, int number, const shared_ptr<chip_t> &chip)
	: selfreference_type(false)
	, m_chip(chip)
	, m_basepath(!path.empty() ? path : make_basepath(*chip, number))
	, m_number(number)
{
	init();
}


void pwm::init()
{
	m_expeption_mask = std::ios::badbit;

	if (m_number == 2 && m_chip && m_chip->quirks()[chip::Quirks::pwm2_alters_pwm1]) {
		m_associated = m_chip->pwm(1);
	}
}


std::string pwm::make_basepath(const chip_t &chip, int number)
{
	if (number <= 0)
		BOOST_THROW_EXCEPTION(std::logic_error("'number' must be positive"));

	std::ostringstream str;
	str << chip.path();
	if (chip.path().last() != '/') str << '/';
	str << Item::prefix() << number;
	return str.str();
}


pwm::value_t pwm::raw_value() const throw (io_error)
{
	const pwm *p = (m_number == 2 && m_chip && m_chip->quirks()[chip::Quirks::pwm2_alters_pwm1]) ?
			META_CHECK_POINTER(m_associated) : this;

	return p->value_read(Item::name(Item::pwm));
}


pwm::rate_t pwm::value() const throw (io_error)
{
	return static_cast<rate_t>(raw_value()) * pwm_max_inverse();
}


pwm::enable_enum pwm::enable(value_t *raw) const throw (io_error)
{
	value_t value = this->value(Item::enable);
	if (raw)
		*raw = value;
	return static_cast<enable_enum>(std::min<value_t>(value, Enable::automatic));
}


pwm::value_t pwm::value(item_enum item) const throw (io_error)
{
	return (item != Item::pwm) ? value_read(Item::name(item)) : raw_value();
}


pwm::value_t pwm::value(const string_ref &item) const throw (io_error)
{
	return !item.empty() ? value_read(item) : raw_value();
}


pwm::value_t pwm::value_read(const string_ref &item, bool ignore_value) const throw (io_error)
{
	std::fstream f;
	f.exceptions(m_expeption_mask);
	open(f, item, std::ios::in);

	value_t value;
	if (!ignore_value) {
		f >> value;
	} else {
		f.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		value = 0;
	}

	return value;
}


void pwm::raw_value(value_t value) throw (io_error)
{
	const string_ref &item = Item::name(Item::pwm);

	if (m_chip && m_chip->quirks()[chip::Quirks::pwm_read_before_write]) {
		value_read(item, true);
	}

	value_write(item, std::min(value, pwm_max()));
}


void pwm::value(rate_t value) throw (io_error)
{
	this->raw_value(static_cast<value_t>(std::max<rate_t>(value, 0) * static_cast<rate_t>(pwm_max()) + 0.5f));
}


void pwm::value(item_enum item, value_t value) throw (io_error)
{
	return (item != Item::pwm) ? value_write(Item::name(item), value) : raw_value(value);
}


void pwm::value(const string_ref &item, value_t value) throw (io_error)
{
	return !item.empty() ? value_write(item, value) : raw_value(value);
}


void pwm::value_write(const string_ref &item, value_t value) throw (io_error)
{
	std::fstream f;
	f.exceptions(m_expeption_mask);
	open(f, item, std::ios::out);
	f << value;
}


std::string pwm::make_itempath(const string_ref &item) const
{
	if (item.empty()) {
		return m_basepath;
	} else {
		std::string dst;
		const size_t required = m_basepath.length() + item.length() + 1;
		dst.reserve(required + 1);
		dst = m_basepath;
		dst += '_';
		item.AppendToString(&dst);
		BOOST_ASSERT(dst.length() == required);
		return dst;
	}
}


void pwm::open(std::fstream &file, const string_ref &item, std::ios::openmode mode) const
{
	file.open(item.empty() ? m_basepath.c_str() : make_itempath(item).c_str(), mode);
}


bool pwm::exists_internal(const string_ref &item, int mode) const
{
	const char *path;

	std::string buf;
	if (item.empty()) {
		path = m_basepath.c_str();
	} else {
		buf = make_itempath(item);
		path = buf.c_str();
	}

	if (::euidaccess(path, mode) == 0)
		return true;

	switch (errno) {
		case EACCES:
		case ELOOP:
		case ENAMETOOLONG:
		case ENOENT:
		case ENOTDIR:
		case EROFS:
			break;

		default:
			BOOST_THROW_EXCEPTION(io_error()
				<< io_error::what_t("::access() failed")
				<< io_error::filename(path)
				<< io_error::errno_code(errno));
			break;
	}
	return false;
}


bool pwm::exists(const string_ref &item, std::ios::open_mode mode_) const
{
	int mode = 0;

	if (mode_ & std::ios::in)
		mode |= R_OK;

	if (mode_ & std::ios::out)
		mode |= W_OK;

	if (!mode)
		BOOST_THROW_EXCEPTION(std::ios::failure("Invalid open mode"));

	return exists_internal(item, mode);
}


bool pwm::exists(item_enum item) const
{
	return exists_internal(Item::name(item), R_OK|W_OK);
}


pwm::rate_t pwm::normalize(rate_t v)
{
	if (v > 1) {
		v *= pwm_max_inverse();
	}
	return v;
}

} /* namespace sensors */
