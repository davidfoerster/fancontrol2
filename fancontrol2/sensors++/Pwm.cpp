/*
 * Pwm.cpp
 *
 *  Created on: 24.12.2011
 *      Author: malte
 */

#include "Pwm.hpp"
#include "Chip.hpp"
#include <boost/filesystem.hpp>
#include <sstream>
#include <algorithm>
#include <limits>
#include <cstring>
#include <cassert>

namespace sensors {


const Pwm::Item::NamesType::ContainerType &Pwm::Item::Names()
{
	static NamesType *ans = new NamesType;
	NamesType::ContainerType &a = ans->container();
	if (a.empty()) {
		const size_t initial_capacity = a.capacity();

		a.push_back(STRING_REF(""));
		a.push_back(STRING_REF("enable"));
		a.push_back(STRING_REF("mode"));
		a.push_back(STRING_REF("freq"));
		a.push_back(STRING_REF("start_output"));
		a.push_back(STRING_REF("stop_output"));
		a.push_back(STRING_REF("target"));
		a.push_back(STRING_REF("tolerance"));
		a.push_back(STRING_REF("auto_channels_temp"));

		assert(a.size() == initial_capacity);
	}
	return a;
}


string_ref Pwm::Item::prefix()
{
	return string_ref::make_const("pwm");
}


void Pwm::init()
{
	//mFile.exceptions(std::ios::badbit | std::ios::failbit);
	mAutoClose = false;
}


Pwm::Pwm(const string_ref &path)
	: mBasePath(path.as_string())
	, mNumber(0)
	, pChip(NULL)
{
	init();
}


Pwm::Pwm(const Chip &chip, int number, bool storeChipReference)
	: mBasePath(makeBasePath(chip, number))
	, mNumber(number)
	, pChip(storeChipReference ? &chip : NULL)
{
	init();
}


Pwm::Pwm(const std::string &path, const Chip &chip, int number, bool storeChipReference)
	: mBasePath(!path.empty() ? path : makeBasePath(chip, number))
	, mNumber(number)
	, pChip(storeChipReference ? &chip : NULL)
{
	init();
}


std::string Pwm::makeBasePath(const Chip &chip, int number)
{
	assert(number > 0);
	string_ref chipPath(chip.getPath());
	std::ostringstream str;
	str << chipPath;
	if (chipPath.last() != '/') str << '/';
	str << Item::prefix() << number;
	return str.str();
}


Pwm::Pwm(const Pwm &other)
	: mBasePath(other.mBasePath)
	, mNumber(other.mNumber)
	, mFile()  // intentionally not copied
	, pChip(other.pChip)
	, mAutoClose(other.mAutoClose)
{
	mFile.exceptions(other.mFile.exceptions());
}


Pwm &Pwm::operator=(const Pwm &other)
{
	closeFiles();
	mFile.exceptions(other.mFile.exceptions());
	mBasePath = other.mBasePath;
	mNumber = other.mNumber;
	pChip = other.pChip;
	mAutoClose = other.mAutoClose;

	return *this;
}


Pwm::~Pwm()
{
	closeFiles();
}


void Pwm::setAutoClose(bool autoClose)
{
	mAutoClose = autoClose;
	if (autoClose)
		closeFiles();
}


void Pwm::openFiles() throw (std::ios::failure)
{
	if (!mFile.is_open()) {
		assert(!mBasePath.empty());
		mFile.open(mBasePath.c_str());
	}
}


void Pwm::closeFiles() throw (std::ios::failure)
{
	if (mFile.is_open())
		mFile.close();
}


Pwm::iostate Pwm::getValue(value_t &raw_value) throw (std::ios::failure)
{
	openFiles();
	mFile >> raw_value;
	iostate state = mFile.rdstate();
	if (mAutoClose) {
		mFile.close();
	} else {
		mFile.seekg(0);
	}
	return state;
}


Pwm::iostate Pwm::getValue(rate_t &value) throw (std::ios::failure)
{
	value_t raw_value;
	iostate state = getValue(raw_value);
	if (state == 0) {
		value = static_cast<rate_t>(raw_value) * pwm_max_inverse();
	}
	return state;
}


Pwm::iostate Pwm::getEnable(EEnable &enable, value_t *raw) throw (std::ios::failure)
{
	value_t raw_value;
	iostate state = getValue(Item::enable, raw_value);
	if (state == 0) {
		enable = static_cast<EEnable>(std::min<value_t>(raw_value, Enable::_length - 1));
		if (raw)
			*raw = raw_value;
	}
	return state;
}


Pwm::iostate Pwm::getValue(EItem item, value_t &value) throw (std::ios::failure)
{
	return (item != Item::pwm) ? getValue_internal(Item::getName(item), value) : getValue(value);
}


Pwm::iostate Pwm::getValue(const string_ref &item, value_t &value) throw (std::ios::failure)
{
	return !item.empty() ? getValue_internal(item, value) : getValue(value);
}


Pwm::iostate Pwm::getValue_internal(const string_ref &item, value_t &value) const throw (std::ios::failure)
{
	std::ifstream f;
	f.exceptions(mFile.exceptions());
	f.open(makeItemPath(item).c_str());
	f >> value;
	f.close();
	return f.rdstate();
}


Pwm::iostate Pwm::setValue(value_t raw_value) throw (std::ios::failure)
{
	if (raw_value > pwm_max()) raw_value = pwm_max();

	iostate state;

	do {
		closeFiles(); mFile.clear();
		errno = 0;
		std::fstream &f = mFile;
		f.open("/tmp/foo");
		if (!f.fail()) {
			f.seekg(0);
			if (!f.fail()) {
				std::string s;
				std::getline(f, s, '\0');
				if (!f.fail()) {
					f.seekg(0);
					//f.seekp(0);
					if (!f.fail()) {
						f.put('0');
						state = f.rdstate();
						if (!f.fail()) {
							f.close();
							if (!f.fail()) {
								break;
							}
						}
					}
				}
			}
		}
		std::cerr << "Error: " << std::strerror(errno) << std::endl;
		std::exit(1);
	} while (false);

	closeFiles();
	//openFiles();
	mFile.open(mBasePath.c_str());
	assert(mFile && mFile.is_open());
	//mFile.seekp(0); state = mFile.rdstate();

	if (pChip && pChip->getQuirks().test(Chip::Quirks::pwm_read_before_write)) {
		mFile.seekg(0); state = mFile.rdstate();
		mFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		state = mFile.rdstate();
		mFile.seekg(0); state = mFile.rdstate();
	}

	mFile << raw_value;
	state = mFile.rdstate();
	if (mAutoClose) {
		mFile.close();
	} else {
		mFile.flush();
		state = mFile.rdstate();
		mFile.seekp(0);
	}
	state = mFile.rdstate();

	return state;
}


Pwm::iostate Pwm::setValue(rate_t value) throw (std::ios::failure)
{
	return setValue(static_cast<value_t>(std::max<rate_t>(value, 0) * static_cast<rate_t>(pwm_max()) + 0.5f));
}


Pwm::iostate Pwm::setValue(EItem item, value_t value) throw (std::ios::failure)
{
	return (item != Item::pwm) ? setValue_internal(Item::getName(item), value) : setValue(value);
}


Pwm::iostate Pwm::setValue(const string_ref &item, value_t value) throw (std::ios::failure)
{
	return !item.empty() ? setValue_internal(item, value) : setValue(value);
}


Pwm::iostate Pwm::setValue_internal(const string_ref &item, value_t value) const throw (std::ios::failure)
{
	std::ofstream f;
	f.exceptions(mFile.exceptions());
	f.open(makeItemPath(item).c_str());
	f << value;
	f.close();
	return f.rdstate();
}


std::string Pwm::makeItemPath(const string_ref &item) const
{
	if (item.empty())
		return mBasePath;

	const size_t required = mBasePath.size() + item.length() + 1;
	std::string path; path.reserve(required + 1);
	path = mBasePath;
	path += '_';
	item.AppendToString(&path);
	assert(path.length() == required);
	return path;
}


bool Pwm::exists(EItem item) throw (std::ios::failure)
{
	if (item != Item::pwm)
		return exists_const(item);

	openFiles();
	bool good = mFile;
	if (mAutoClose)
		closeFiles();
	return good;
}


bool Pwm::exists_const(const string_ref &item) const throw (std::ios::failure)
{
	return !item.empty() && boost::filesystem::exists(makeItemPath(item));
}


bool Pwm::exists_const(EItem item) const throw (std::ios::failure)
{
	return exists_const(Item::getName(item));
}


Pwm::rate_t Pwm::normalize(rate_t v)
{
	if (v > 1) {
		v *= pwm_max_inverse();
	}
	return v;
}

} /* namespace sensors */
