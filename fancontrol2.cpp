/*
 * fancontrol2.cpp
 *
 *  Created on: 14.12.2011
 *      Author: malte
 */

#include "utils.hpp"
#include "fan.hpp"
#include "meta/utility.hpp"

#include <cstdlib>

namespace fancontrol {


int main(int argc, char *argv[])
{
	int r = EXIT_SUCCESS;
	std::auto_ptr<fancontrol::config_wrapper> cfg_wrap;

	try {
		cfg_wrap = fancontrol::config_wrapper::make_config(argc, argv);
		config::fans_container &fans = META_CHECK_POINTER(cfg_wrap)->cfg.fans;

		if (!cfg_wrap->do_check) {
			register_signal_handlers();

			do {
				for (config::fans_container::iterator it = fans.begin(); it != fans.end(); ++it) {
					META_CHECK_POINTER(*it)->update_valve();
				}
			} while ((r = sleep(&cfg_wrap->interval)) < 0);

			cfg_wrap.reset();

		} else {
			r = !fans.empty() ? EXIT_SUCCESS : EXIT_FAILURE;
		}
	} catch (meta::exception_base &e) {
		r = handle_exception(e, !!cfg_wrap);
	} catch (std::runtime_error &e) {
		r = handle_exception(e, !!cfg_wrap);
	}

	return r;
}

}


int main(int argc, char *argv[])
{
	return fancontrol::main(argc, argv);
}

