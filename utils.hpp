/*
 * utils.hpp
 *
 *  Created on: 31.01.2012
 *      Author: malte
 */

#pragma once
#ifndef FANCONTROL_UTILS_HPP_
#define FANCONTROL_UTILS_HPP_

#include "config.hpp"
#include <exception>

#include <memory>
#include <ctime>
#include <cstdio>

namespace fancontrol {

	using ::sensors::sensor_error;


	void strerror_wrapper(const char *msg = 0, error_t e = 0);


	int handle_exception(std::exception &e, bool cfg_ok);


	int handle_exception(::util::exception_base &e, bool cfg_ok);


	void register_signal_handlers();


	int sleep(const struct timespec *duration);


	class config_wrapper {
	public:
		config_wrapper(
			std::ifstream &config_file,
			const boost::shared_ptr< ::sensors::sensor_container > &sensors,
			bool do_check)
				throw(::util::runtime_error, ::YAML::ParserException, std::ios::failure);

		static std::unique_ptr<config_wrapper> make_config(int argc, char *argv[])
				throw(::util::runtime_error, ::YAML::ParserException);

		config cfg;

		struct timespec interval;

		const bool do_check;
	};

}

#endif /* FANCONTROL_UTILS_HPP_ */
