/*
 * utils.cpp
 *
 *  Created on: 31.01.2012
 *      Author: malte
 */

#include "utils.hpp"
#include "sensors++/sensors.hpp"
#include "util/assert.hpp"

#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include <iostream>
#include <fstream>

#include <cerrno>
#include <cstring>
#include <csignal>
#include <cstdlib>

namespace fancontrol {

	using ::sensors::sensor_container;
	using ::sensors::sensor_error;

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


	int handle_exception(::util::exception_base &e, bool cfg_ok)
	{
		return handle_exception(
				static_cast< std::exception&>(e),
				cfg_ok
	#if FANCONTROL_PIDFILE
				|| !!dynamic_cast< ::util::pidfile_exception*>(&e)
	#endif
			);
	}


	static int last_signal;

	void handle_signal(int signal)
	{
		last_signal = signal;
	}


	const struct sigaction *get_signal_action_definition()
	{
		struct sigaction *signal_action_definition = new struct sigaction();
		signal_action_definition->sa_handler = &handle_signal;
		return signal_action_definition;
	}


	void register_signal_handlers()
	{
		static const struct sigaction *signal_action_definition = get_signal_action_definition();
		static const int signals[] = { SIGHUP, SIGINT, SIGQUIT, SIGPIPE, SIGTERM, SIGCONT };

		last_signal = -1;

		BOOST_FOREACH(const int signal, signals) {
			BOOST_VERIFY_P(::sigaction(signal, signal_action_definition, 0) == 0);
		}
	}


	int sleep_reset()
	{
		const int return_value = -last_signal;
		last_signal = -1;
		return return_value;
	}


	int sleep(const struct timespec *duration)
	{
		if (::nanosleep(duration, 0) != 0) {
			if (errno == EINTR) {
				errno = 0;
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
						return sleep_reset();

					default:  // i.e. SIGPIPE
						UTIL_DEBUG(std::cerr << "Interrupted by signal " << last_signal << std::endl);
						break;
				}
				// reset and leave with non-zero return value
				return 2;

			} else {
				strerror_wrapper("'nanosleep' error");
				return EXIT_FAILURE;
			}
		} else {
			// continue normally
			return -1;
		}
	}


	config_wrapper::config_wrapper(
		std::ifstream &config_file, const boost::shared_ptr<sensor_container> &sens, bool do_check)
			throw(::util::runtime_error, ::YAML::ParserException, std::ios::failure)
		: cfg(config_file, sens, do_check)
		, do_check(do_check)
	{
		cfg.interval(&interval);
	}


	std::unique_ptr<config_wrapper> config_wrapper::make_config(int argc, char *argv[])
			throw(::util::runtime_error, ::YAML::ParserException)
	{
		static const char *default_cfg_filename = UTIL_STRING(FANCONTROL_CONFIGFILE);
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
			cerr << "Ignoring " << (argc - argp) << " superflous command line arguments:";
			do {
				cerr << ' ' << argv[argp];
			} while (++argp < argc);
			cerr << endl;
		}


		try {
			std::ifstream cfg_file;
			cfg_file.exceptions(std::ios::badbit);
			cfg_file.open(cfg_filename);

			return std::unique_ptr<config_wrapper>(
					new config_wrapper(cfg_file, boost::make_shared<sensor_container>(), do_check));
		} catch (std::ios::failure &e) {
			using ::util::io_error;
			BOOST_THROW_EXCEPTION(io_error()
				<< io_error::what_t(e.what())
				<< io_error::errno_code(errno)
				<< io_error::filename(cfg_filename));
		}
	}

}
