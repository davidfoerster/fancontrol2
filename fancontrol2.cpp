/*
 * fancontrol2.cpp
 *
 *  Created on: 14.12.2011
 *      Author: malte
 */

#include "utils.hpp"
#include "fan.hpp"
#include <boost/foreach.hpp>
#include <cstdlib>
#include <csignal>


namespace fancontrol {

	int main(int argc, char *argv[])
	{
		int r = EXIT_SUCCESS;
		std::unique_ptr<fancontrol::config_wrapper> cfg_wrap;

		try {
			cfg_wrap = fancontrol::config_wrapper::make_config(argc, argv);
			config::fans_container &fans = cfg_wrap->cfg.fans;

			if (!cfg_wrap->do_check) {
				register_signal_handlers();

				do {
					BOOST_FOREACH(shared_ptr<fan> &f, fans){
						const bool force = r == -SIGCONT;
						f->update_valve(force);
					}
				} while ((r = sleep(&cfg_wrap->interval)) < 0);

				cfg_wrap.reset();

			} else {
				r = !fans.empty() ? EXIT_SUCCESS : EXIT_FAILURE;
			}
		} catch (util::exception_base &e) {
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

