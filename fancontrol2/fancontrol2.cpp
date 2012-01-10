/*
 * fancontrol2.cpp
 *
 *  Created on: 14.12.2011
 *      Author: malte
 */

#include "sensors++/sensors.hpp"
#include "Config.hpp"
#include <fstream>
#include <ctime>
#include <cerrno>
#include <cstring>


namespace fancontrol {

using namespace sensors;

using std::cout;
using std::cerr;
using std::endl;


void strerror(const char *msg = NULL, error_t e = 0)
{
	if (msg) {
		cerr << msg << ':' << ' ';
	}

	if (!e) e = errno;
	const char *msg2 = std::strerror(e);
	if (msg2 && *msg) {
		cerr << msg2;
	} else {
		cerr << "Unknown error " << e;
	}

}


void handleException(std::exception &e, bool cfg_ok, int &r) throw(std::exception)
{
	if (cfg_ok) {
		r = 1;
	} else {
		r = 2;
		cerr << "Error while parsing the configuration file.\n";
	}

	cerr << "  what(): " << e.what();
	strerror("");
	cerr << endl;
	//throw e;
}


void handleException(SensorError &e, bool cfg_ok, int &r) throw(SensorError)
{
	if (cfg_ok) {
		r = 1;
	} else {
		r = 2;
		cerr << "Error while parsing the configuration file.\n";
	}

	cerr << "  what(): " << e.what() << endl;
	//throw e;
}


int main(int argc, char *argv[])
{
	int r = 0;

	std::ifstream cfg_file;
	cfg_file.exceptions(std::ios::badbit);

	bool cfg_ok = false;
	try {
		cfg_file.open("fancontrol2.yaml");

		Sensors sens;
		Config cfg(cfg_file, sens);
		if (cfg.fans.size() == 0)
			throw SensorError("No fans selected");

		struct timespec interval; cfg.getInterval(&interval);
		cfg_ok = true;

		do {
			for (std::vector<Fan>::iterator it = cfg.fans.begin(); it != cfg.fans.end(); ++it) {
				it->updateValve();
			}
		} while (nanosleep(&interval, NULL) == 0);

		if (errno != EINTR) {
			r = 1;
			strerror("`nanosleep' error");
		}
	} catch (SensorError &e) {
		handleException(e, cfg_ok, r);
	} catch (std::exception &e) {
		handleException(e, cfg_ok, r);
	}

	cfg_file.close();

	return r;
}

}


int main(int argc, char *argv[])
{
	return fancontrol::main(argc, argv);
}
