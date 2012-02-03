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


#ifndef FANCONTROL_CONFIGFILE
#	ifdef NDEBUG
#		define FANCONTROL_CONFIGFILE /etc/fancontrol2.yaml
#	else
#		define FANCONTROL_CONFIGFILE fancontrol2.yaml
#	endif
#endif


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
						// no error, but request reset and leave
						return EXIT_SUCCESS;

					case SIGCONT:
						// request to poll now (instead of waiting a whole interval)
						last_signal = -1;
						errno = 0;
						return -1;

					default:  // i.e. SIGPIPE
						META_DEBUG(std::cerr << "Interrupted by signal " << last_signal << std::endl);
						break;
				}
				// reset and leave with non-zero return value
				return 2;

			default:
				strerror_wrapper("'nanosleep' error");
				return EXIT_FAILURE;
		}
	}
	// continue normally
	return -1;
}


config_wrapper::config_wrapper(
	std::ifstream &config_file, const boost::shared_ptr<sensor_container> &sens, bool do_check)
		throw(meta::runtime_error, YAML::ParserException, std::ios::failure)
	: cfg(config_file, sens, do_check)
	, do_check(do_check)
{
	cfg.interval(&interval);
}


std::auto_ptr<config_wrapper> config_wrapper::make_config(int argc, char *argv[])
		throw(meta::runtime_error, YAML::ParserException)
{
	static const char *default_cfg_filename = META_STRING(FANCONTROL_CONFIGFILE);
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

	if (argp < argc) {
		std::cerr << "Ignoring " << (argc - argp) << " superflous command line arguments:";
		do {
			std::cerr << ' ' << argv[argp];
		} while (++argp < argc);
		std::cerr << std::endl;
	}


	try {
		std::ifstream cfg_file;
		cfg_file.exceptions(std::ios::badbit);
		cfg_file.open(cfg_filename);

		return std::auto_ptr<config_wrapper>(new config_wrapper(cfg_file, boost::make_shared<sensor_container>(), do_check));
	} catch (std::ios::failure &e) {
		throw meta::io_error()
			<< meta::io_error::what_t(e.what())
			<< meta::io_error::errno_code(errno)
			<< meta::io_error::filename(cfg_filename);
	}
}

}
