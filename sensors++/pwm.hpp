/*
 * pwm.hpp
 *
 *  Created on: 24.12.2011
 *      Author: malte
 */

#pragma once
#ifndef SENSORS_PWM_HPP_
#define SENSORS_PWM_HPP_

#include "internal/common.hpp"
#include "../util/static_allocator/static_string.hpp"
#include "exceptions.hpp"

#include <memory>
#include <string>
#include <array>


namespace sensors {

	using std::shared_ptr;
	using util::io_error;

	class chip;


	class pwm
		: virtual public std::enable_shared_from_this<pwm>
	{
	public:
		typedef sensors::chip chip_t;

		typedef unsigned value_t;

		typedef float rate_t;

		typedef std::ios::iostate iostate;

		struct Item {
			enum value {
				pwm = 0,
				enable,
				mode,
				freq,
				start_output,
				stop_output,
				target,
				tolerance,
				auto_channels_temp,
				_length
			};

			static const string_ref &prefix();

			typedef std::array<string_ref, _length> names_type;

			static const names_type &names();

			static const string_ref &name(value what);
		};

		typedef Item::value item_enum;

		struct Mode {
			enum ModeEnum {
				direct_current = 0,
				pwm = 1,
				_length
			};
		};

		typedef Mode::ModeEnum mode_enum;

		struct Enable {
			enum EnableEnum {
				off = 0,
				manual = 1,
				automatic = 2,
				_length
			};
		};

		typedef Enable::EnableEnum enable_enum;

		static constexpr unsigned pwm_max();

		static constexpr rate_t pwm_max_inverse();

		static constexpr rate_t normalize(rate_t v);

		static rate_t *normalize(rate_t *v);

		static std::string make_basepath(const chip_t &chip, int number);

		pwm(const string_ref &path);

		pwm(int number, const shared_ptr<chip_t> &chip);

		bool exists(item_enum item = Item::pwm) const;

		bool exists(const string_ref &item, std::ios::open_mode mode = std::ios::in) const;

		value_t raw_value() const throw (io_error);

		rate_t value() const throw (io_error);

		enable_enum enable(value_t *raw = 0) const throw (io_error);

		value_t value(item_enum item) const throw (io_error);

		value_t value(const string_ref &item) const throw (io_error);

		void raw_value(value_t raw_value) throw (io_error);

		void value(rate_t value) throw (io_error);

		void value(item_enum item, value_t value) throw (io_error);

		void value(const string_ref &item, value_t value) throw (io_error);

		const std::string &path() const;

		int number() const;

		shared_ptr<const chip_t> chip() const;

		bool operator==(const pwm &other) const;

		iostate m_expeption_mask;

	protected:
		void init();

		shared_ptr<chip_t> m_chip;

		shared_ptr<pwm> m_associated;

		std::string m_basepath;

		int m_number;

	private:
		value_t value_read(const string_ref &item, bool ignore_value = false) const throw (io_error);

		void value_write(const string_ref &item, value_t value) throw (io_error);

		bool exists_internal(const string_ref &item, int open_mode) const;

		typedef util::static_string<1 << 8> itempath_buffer_type;
		const char *make_itempath(const string_ref &item, itempath_buffer_type &dst) const;

		void open(std::fstream &file, const string_ref &item, std::ios::openmode mode) const;

		friend class sensors::chip;
	};



// implementations ========================================


	inline
	const string_ref &pwm::Item::name(value what)
	{
		return names()[what];
	}


	inline constexpr
	unsigned pwm::pwm_max()
	{
		return 255;
	}

	inline constexpr
	pwm::rate_t pwm::pwm_max_inverse()
	{
		return 1.f / static_cast<rate_t>(pwm_max());
	}


	inline
	pwm::rate_t *pwm::normalize(rate_t *v)
	{
		*v = normalize(*v);
		return v;
	}


	inline constexpr
	pwm::rate_t pwm::normalize(rate_t v)
	{
		return (v <= 1.f) ? v : (v * pwm_max_inverse());
	}


	inline
	const std::string &pwm::path() const
	{
		return m_basepath;
	}


	inline
	int pwm::number() const
	{
		return m_number;
	}


	inline
	shared_ptr<const chip> pwm::chip() const
	{
		return m_chip;
	}


	inline
	bool pwm::operator==(const pwm &other) const
	{
		return this == &other || this->m_basepath == other.m_basepath;
	}

} /* namespace SENSORS_ */
#endif /* SENSORS_PWM_HPP_ */
