/*
 * Pwm.cpp
 *
 *  Created on: 24.12.2011
 *      Author: malte
 */

#include "pwm.hpp"
#include "chip.hpp"
#include "util/strcat.hpp"
#include "util/algorithm.hpp"
#include "util/yaml.hpp"
#include <boost/range/algorithm/copy.hpp>
#include <boost/integer/static_log2.hpp>
#include <boost/assert.hpp>
#include <fstream>
#include <algorithm>
#include <limits>
#include <type_traits>
#include <stdexcept>
#include <cstring>
#include <cstdint>
#include <unistd.h>


namespace sensors {

	using std::shared_ptr;
	using util::io_error;


	pwm::pwm(const string_ref &path)
		: m_chip()
		, m_basepath(path.str())
		, m_number(0)
	{
		init();
	}


	pwm::pwm(int number, const shared_ptr<chip_t> &chip)
		: m_chip(chip)
		, m_basepath(make_basepath(*chip, number))
		, m_number(number)
	{
		init();
	}


	const pwm::Item::names_type &pwm::Item::names()
	{
		static names_type &a = *new names_type({{
				STRING_REF(""),
				STRING_REF("enable"),
				STRING_REF("mode"),
				STRING_REF("freq"),
				STRING_REF("start_output"),
				STRING_REF("stop_output"),
				STRING_REF("target"),
				STRING_REF("tolerance"),
				STRING_REF("auto_channels_temp")
			}});
		return a;
	}


	const string_ref &pwm::Item::prefix()
	{
		const static string_ref &pwm = *new string_ref(STRING_REF("pwm"));
		return pwm;
	}


	/*
	pwm::pwm(const std::string &path, int number, const shared_ptr<chip_t> &chip)
		: selfreference_type(false)
		, m_chip(chip)
		, m_basepath(!path.empty() ? path : make_basepath(*chip, number))
		, m_number(number)
	{
		init();
	}
	*/


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

		const string_ref &path = chip.path(), &prefix = Item::prefix();
		std::string str;
		str.reserve(path.size() + prefix.size() + 4);
		path.str(str);
		if (str.back() != '/')
			str += '/';
		return (str += prefix) << static_cast<unsigned int>(number);
	}


	pwm::value_t pwm::raw_value() const throw (io_error)
	{
		const pwm *p = (m_number == 2 && m_chip && m_chip->quirks()[chip::Quirks::pwm2_alters_pwm1]) ?
				UTIL_CHECK_POINTER(m_associated) : this;

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
			f.ignore(std::numeric_limits< std::streamsize>::max(), '\n');
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


	const char *pwm::make_itempath(const string_ref &item, itempath_buffer_type &dst) const
	{
		if (item.empty()) {
			return m_basepath.c_str();
		} else {
			const std::size_t required = m_basepath.length() + item.length() + 1;
			dst.reserve(required + 1);
			dst = m_basepath;
			(dst += '_') += item;
			BOOST_ASSERT(dst.length() == required);
			return dst.c_str();
		}
	}


	void pwm::open(std::fstream &file, const string_ref &item, std::ios::openmode mode) const
	{
		itempath_buffer_type buf;
		file.open(make_itempath(item, buf), mode);
	}


	bool pwm::exists_internal(const string_ref &item, int mode) const
	{
		static const int acceptable_errnos[] = {
			EACCES, ELOOP, ENAMETOOLONG, ENOENT, ENOTDIR, EROFS
		};

		itempath_buffer_type buf;
		const char *const path = make_itempath(item, buf);
		if (::euidaccess(path, mode) == 0)
			return true;

		if (util::any_of_equal(acceptable_errnos, errno))
			return false;

		BOOST_THROW_EXCEPTION(io_error()
			<< io_error::what_t("::access() failed")
			<< io_error::filename(path)
			<< io_error::errno_code(errno));
	}


	namespace helper {

		template <int N, typename Integer>
		inline static typename std::enable_if< N >= 0, Integer>::type shift_left(const Integer &x)
		{
			return x << N;
		}

		template <int N, typename Integer>
		inline static typename std::disable_if< N >= 0, Integer>::type shift_left(const Integer &x)
		{
			typedef typename std::make_unsigned<Integer>::type Unsigned;
			return static_cast<Integer>(static_cast<Unsigned>(x) >> -N);
		}


		template <uintmax_t Source, uintmax_t Dest, typename Integer>
		inline static Integer convert_flagbit(const Integer &src)
		{
			using boost::static_log2;
			return shift_left<static_log2<Dest>::value - static_log2<Source>::value>(src & static_cast<Integer>(Source));
		}

	}


	bool pwm::exists(const string_ref &item, std::ios::open_mode mode_) const
	{
		#if F_OK != 0
		#	error "Assumption violated"
		#endif

		const int mode = F_OK
			| helper::convert_flagbit< std::ios::in, R_OK, int >(mode_)
			| helper::convert_flagbit< std::ios::out, W_OK, int >(mode_)
			;

		return exists_internal(item, mode);
	}


	bool pwm::exists(item_enum item) const
	{
		return exists_internal(Item::name(item), R_OK|W_OK);
	}

} /* namespace sensors */
