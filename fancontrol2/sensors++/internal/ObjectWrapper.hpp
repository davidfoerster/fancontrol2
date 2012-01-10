/*
 * ObjectWrapper.hpp
 *
 *  Created on: 20.12.2011
 *      Author: malte
 */

#pragma once
#ifndef SENSORS_OBJECTWRAPPER_HPP_
#define SENSORS_OBJECTWRAPPER_HPP_

#include "../Lock.hpp"
#include "meta/GloballySharedResource.hpp"
#include <stdexcept>

namespace sensors {


class InvalidOperationException: public std::logic_error
{
public:
	InvalidOperationException(const std::string &what) : std::logic_error(what) {}
};


template <typename T, typename _Parent = void>
class ObjectWrapper
{
public:
	//T *get() { return object.get(); }
	const T *get() const { return object.get(); }

	//T *operator->() { return get(); }
	const T *operator->() const { return get(); }

	//T &operator*() { return *get(); }
	const T &operator*() const { return *get(); }

	const _Parent *getParent() const { return parent; }

protected:
	ObjectWrapper(T *object, _Parent *parent = 0)
	:	parent(parent)
	,	object(Lock::instance().makePtr<T>(object))
	{
	}

	_Parent *parent;

	meta::GloballySharedResource::ptr<T> object;
};

} /* namespace sensors */
#endif /* SENSORS_OBJECTWRAPPER_HPP_ */
