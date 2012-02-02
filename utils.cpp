/*
 * utils.cpp
 *
 *  Created on: 31.01.2012
 *      Author: malte
 */

#include "utils.hpp"
#include "sensors++/sensors.hpp"
#include "meta/utility.hpp"
#include "meta/assert.hpp"

#include <boost/make_shared.hpp>
#include <iostream>
#include <fstream>

#include <cerrno>
#include <cstring>
#include <csignal>
#include <cstdlib>

namespace fancontrol {

using meta::elementsof;

using sensors::sensor_container;
using sensors::sensor_error;

using std::cout;
using std::cerr;
using std::endl;


void strerror_wrapper(const char *msg, error_t errnum)
{
	if (msg) {
		cerr << msg << ':' << ' ';
	}

	if (!errnum) errnum = errno;
	const char *msg2 = std::strerror(errnum);
	if (msg2 && *msg2) {
		cerr << msg2;
	} else {
		cerr << "Unknown error " << errnum;
	}

	cerr << endl;
}


int handle_exception(std::exception &e, bool cfg_ok)
{
	int r;
	if (cfg_ok) {
		r = 1;
	} else {
		r = 2;
		cerr << "Error while parsing the configuration file: ";
	}

	cerr << e.what() << endl;

	return r;
}


int handle_exception(meta::exception_base &e, bool cfg_ok)
{
	return handle_exception(
			static_cast<std::exception&>(e),
			cfg_ok || !!dynamic_cast<meta::pidfile_exception*>(&e));
}


int last_signal;

void handle_signal(int signal)
{
	last_signal = signal;
}


const struct sigaction *get_signal_action_definition()
{
	static struct sigaction *signal_action_definition = new struct sigaction[1]();
	signal_action_definition->sa_handler = &handle_signal;
	return signal_action_definition;
}


void register_signal_handlers()
{
	static const struct sigaction *signal_action_definition = get_signal_action_definition();
	static const int signals[] = { SIGHUP, SIGINT, SIGQUIT, SIGPIPE, SIGTERM, SIGCONT };

	last_signal = -1;

	for (unsigned i = 0; i < elementsof(signals); i++)
		BOOST_VERIFY_P(::sigaction(signals[i], signal_action_definition, 0) == 0);
}


int sleep(const struct timespec *duration)
{
	if (::nanosleep(duration, 0) != 0) {
		switch (errno) {
			case EINTR:
				BOOST_ASSERT(last_signal >= 0);
				switch (last_signal) {
					case SIGHUP:
					case SIGINT:
					case SIGQUIT:
					case SIGTERM:
						return -1;

					case SIGCONT:
						last_signal = -1;
						errno = 0;
						return EXIT_SUCCESS;

					default:
	#ifndef NDEBUG
						std::cerr << "Interrupted by signal " << last_signal << '.' << std::endl;
	#endif
						break;
				}
				return 2;

			default:
				strerror_wrapper("'nanosleep' error");
				return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}


data::data(std::ifstream &config_file, const boost::shared_ptr<sensor_container> &sens, bool do_check)
		throw(meta::runtime_error, YAML::ParserException, std::ios::failure)
	: cfg(config_file, sens, do_check)
	, do_check(do_check)
{
	cfg.interval(&interval);
}


std::auto_ptr<data> data::make_config(int argc, char *argv[])
		throw(meta::runtime_error, YAML::ParserException)
{
	static const char *default_cfg_filename =
#ifdef NDEBUG
				"/etc/fancontrol2.yaml"
#else
				"fancontrol2.yaml"
#endif
			;
	int argp = 1;
	const char *cfg_filename = default_cfg_filename;
	bool do_check = false;

	if (argp < argc && std::strcmp(argv[argp], "--check") == 0) {
		argp += 1;
		do_check = true;
	}

	if (argp < argc) {
		cfg_filename = argv[argp++];
	}


	try {
		std::ifstream cfg_file;
		cfg_file.exceptions(std::ios::badbit);
		cfg_file.open(cfg_filename);

		return std::auto_ptr<data>(new data(cfg_file, boost::make_shared<sensor_container>(), do_check));
	} catch (std::ios::failure &e) {
		throw meta::io_error()
			<< meta::io_error::what_t(e.what())
			<< meta::io_error::errno_code(errno)
			<< meta::io_error::filename(cfg_filename);
	}
}

}
