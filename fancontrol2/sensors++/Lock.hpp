/*
 * Lock.hpp
 *
 *  Created on: 19.12.2011
 *      Author: malte
 */

#pragma once
#ifndef SENSORS_LOCK_HPP_
#define SENSORS_LOCK_HPP_

#include "meta/GloballySharedResource.hpp"
#include <cstdio>

namespace sensors {

template <typename, typename> class ObjectWrapper;
class Sensors;
class Chip; struct sensors_chip_name;
class Feature; struct sensors_feature;
struct sensors_subfeature;


class Lock: private meta::GloballySharedResource
{
	typedef meta::GloballySharedResource super;

public:
	virtual ~Lock();

	bool init(std::FILE *config = NULL);

	size_t getReferenceCount() const { return super::getReferenceCount(); }

	bool isLocked() const { return super::isValid(); }

	const std::string &getName() const { return super::getName(); }

	bool doesAutoRelease() const { return super::doesAutoRelease(); }

	void setAutoRelease(bool autoRelease) { super::setAutoRelease(autoRelease); }

	bool isInitialized() const { return mInitialized; }

protected:
	Lock();

	virtual void release();

	bool mInitialized;

private:
	static Lock &instance();

	friend class Sensors;
	friend class ObjectWrapper<const Sensors, void>;
	friend class ObjectWrapper<const sensors_chip_name, void>;
	friend class ObjectWrapper<const sensors_feature, Chip>;
	friend class ObjectWrapper<const sensors_subfeature, Feature>;
};

} /* namespace sensors */
#endif /* SENSORS_LOCK_HPP_ */
