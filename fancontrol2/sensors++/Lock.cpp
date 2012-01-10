/*
 * Lock.cpp
 *
 *  Created on: 19.12.2011
 *      Author: malte
 */

#include "Lock.hpp"

namespace sensors {
#include <sensors/sensors.h>


Lock &Lock::instance()
{
	static Lock ans;
	return ans;
}


Lock::Lock()
	: meta::GloballySharedResource("Sensors++ lock")
	, mInitialized(false)
{
}


Lock::~Lock()
{
	if (isReadyForRelease()) {
		Lock::release();
	}
}


bool Lock::init(std::FILE *config)
{
	if (!mInitialized) {
		mInitialized = (sensors_init(config) == 0);
	}
	return mInitialized;
}


void Lock::release()
{
	if (mInitialized) {
		mInitialized = false;
		sensors_cleanup();
	}
}

} /* namespace sensors */
