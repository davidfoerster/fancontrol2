/*
 * Pwm.hpp
 *
 *  Created on: 24.12.2011
 *      Author: malte
 */

#pragma once
#ifndef SENSORS_PWM_HPP_
#define SENSORS_PWM_HPP_

#include "common.hpp"
#include <chromium/base/stack_container.h>
#include <string>
#include <fstream>
#include "csensors.hpp"

namespace sensors {

class Chip;


class Pwm {
public:
	typedef unsigned value_t;

	typedef float rate_t;

	typedef std::ios::iostate iostate;

	struct Item {
	public:
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

		static string_ref prefix();

		typedef chromium::StackVector<string_ref, _length> NamesType;

		static const NamesType::ContainerType &Names();

		static inline const string_ref &getName(value what) { return Names()[what]; }
	};

	typedef Item::value EItem;

	struct Mode {
		enum ModeEnum {
			direct_current = 0,
			pwm = 1,
			_length
		};
	};

	typedef Mode::ModeEnum EMode;

	struct Enable {
		enum EnableEnum {
			off = 0,
			manual = 1,
			automatic = 2,
			_length
		};
	};

	typedef Enable::EnableEnum EEnable;

	static unsigned pwm_max() { return 255; }

	static rate_t pwm_max_inverse() { return 1.f / static_cast<rate_t>(pwm_max()); }

	static rate_t normalize(rate_t v);

	static inline void normalize(rate_t *v) { *v = normalize(*v); }

	static std::string makeBasePath(const Chip &chip, int number);

	template <class T>
	struct PathComparator {
		static int compare(const Pwm &o1, const T &o2) {
			return o1.getPath().compare(o2);
		}
	};

	Pwm(const string_ref &path);

	Pwm(const Chip &chip, int number, bool storeChipReference = false);

	Pwm(const Pwm &other);

	Pwm &operator=(const Pwm &other);

	~Pwm();

	bool exists(EItem item = Item::pwm) throw (std::ios::failure);
	bool exists_const(EItem item) const throw (std::ios::failure);
	bool exists_const(const string_ref &item) const throw (std::ios::failure);

	iostate getValue(value_t &raw_value) throw (std::ios::failure);

	iostate getValue(rate_t &value) throw (std::ios::failure);

	iostate getEnable(EEnable &enable, value_t *raw) throw (std::ios::failure);

	iostate getValue(EItem item, value_t &value) throw (std::ios::failure);

	iostate getValue(const string_ref &item, value_t &value) throw (std::ios::failure);

	iostate setValue(value_t raw_value) throw (std::ios::failure);

	iostate setValue(rate_t value) throw (std::ios::failure);

	iostate setValue(EItem item, value_t value) throw (std::ios::failure);

	iostate setValue(const string_ref &item, value_t value) throw (std::ios::failure);

	const std::string &getPath() const { return mBasePath; }

	int getNumber() const { return mNumber; };

	void openFiles() throw (std::ios::failure);

	void closeFiles() throw (std::ios::failure);

	bool getAutoClose() const { return mAutoClose; }

	void setAutoClose(bool);

	const Chip *getChip() const { return pChip; }

	bool operator==(const Pwm &other) const { return this == &other || this->mBasePath == other.mBasePath; }
	bool operator!=(const Pwm &other) const { return !this->operator==(other); }

protected:
	void init();

	std::string mBasePath;

	int mNumber;

	std::fstream mFile;

private:
	iostate getValue_internal(const string_ref &item, value_t &value) const throw (std::ios::failure);

	iostate setValue_internal(const string_ref &item, value_t value) const throw (std::ios::failure);

	std::string makeItemPath(const string_ref &item) const;

	Pwm(const std::string &pwmPath, const Chip &chip, int number, bool storeChipReference = false);

	const Chip *pChip;

	bool mAutoClose;

	friend class Chip;
};


} /* namespace SENSORS_ */
#endif /* SENSORS_PWM_HPP_ */
