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
	std::auto_ptr<fancontrol::data> data;

	try {
		data = fancontrol::data::make_config(argc, argv);
		config::fans_container &fans = META_CHECK_POINTER(data)->cfg.fans;

		if (!data->do_check) {
			register_signal_handlers();

			do {
				for (config::fans_container::iterator it = fans.begin(); it != fans.end(); ++it) {
					META_CHECK_POINTER(*it)->update_valve();
				}
			} while ((r = sleep(&data->interval)) == EXIT_SUCCESS);

			if (r < 0)
				r = EXIT_SUCCESS;

		} else {
			r = !fans.empty() ? EXIT_SUCCESS : EXIT_FAILURE;
		}

		data.reset();
	} catch (meta::exception_base &e) {
		r = handle_exception(e, !!data);
	} catch (std::runtime_error &e) {
		r = handle_exception(e, !!data);
	}

	return r;
}

}


int main(int argc, char *argv[])
{
	return fancontrol::main(argc, argv);
}

