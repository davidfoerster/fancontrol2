/*
 * GloballySharedResource.cpp
 *
 *  Created on: 16.12.2011
 *      Author: malte
 */

#include "GloballySharedResource.hpp"
#include <boost/format.hpp>
#include <iomanip>
#include <typeinfo>

#ifndef NDEBUG
#	include <iostream>
#	include <csignal>
#endif

namespace meta {


const char *InvalidatedResourceException::descriptionTemplate()
{
	return "Cannot access invalidated resource `%1%'.";
}


InvalidatedResourceException::InvalidatedResourceException(const string &resourceName)
	throw()
	: std::logic_error(makeDescription(resourceName))
{
}


std::string InvalidatedResourceException::makeDescription(const string &resourceName)
{
	boost::format format(descriptionTemplate());
	if (!resourceName.empty()) {
		format % resourceName;
	} else {
		format % "<unknown>";
	}
	return boost::str(format);
}


InvalidatedResourceException::InvalidatedResourceException(const void *resource)
	throw()
	: std::logic_error(makeDescription(resource))
{
}


std::string InvalidatedResourceException::makeDescription(const void *resource)
{
	boost::format format(descriptionTemplate());
	if (resource != NULL) {
		format % boost::io::group(std::hex, std::showbase, std::setw(16+2), resource);
	} else {
		format % "<unknown>";
	}
	return boost::str(format);
}


GloballySharedResource::GloballySharedResource(const char *name, bool autoRelease)
	: mName(name ? name : makeDefaultName())
	, mReferenceCount(0)
	, mInvalid(false)
	, mAutoRelease(autoRelease)
{
}


GloballySharedResource::GloballySharedResource(const string &name, bool autoRelease)
	: mName(!name.empty() ? name : makeDefaultName())
	, mReferenceCount(0)
	, mInvalid(false)
	, mAutoRelease(autoRelease)
{
}


std::string GloballySharedResource::makeDefaultName()
{
	using boost::format;
	using boost::str;

	static const format formatter("%|1$s|(%|2$#018x|)");
	return str(format(formatter) % typeid(this).name() % static_cast<void*>(this));
}


GloballySharedResource::~GloballySharedResource()
{
	if (GloballySharedResource::isReadyForRelease()) {
		invalidate();
	}
}


bool GloballySharedResource::isReadyForRelease()
{
	return !(mAutoRelease && mReferenceCount == 0);
}


void GloballySharedResource::ref()
	throw(InvalidatedResourceException)
{
	checkInvalid();
	mReferenceCount += 1;
}


void GloballySharedResource::unref()
{
#ifndef NDEBUG
	if (mReferenceCount == 0) {
		std::clog << "Potential programming problem: Tried to lower the reference count on `" << mName << "' below 0.";
		raise(SIGINT);
	}
#endif

	if (mReferenceCount == 1 && mAutoRelease) {
		release_internal();
	}

	if (mReferenceCount != 0) {
		mReferenceCount -= 1;
	}
}


void GloballySharedResource::setAutoRelease(bool autoRelease)
{
	if (autoRelease && !mAutoRelease && mReferenceCount == 0) {
		release_internal();
	}
	mAutoRelease = autoRelease;
}


void GloballySharedResource::release_internal()
{
	invalidate();
	release();
}


void GloballySharedResource::checkInvalid() const
	throw(InvalidatedResourceException)
{
	if (mInvalid)
		throw InvalidatedResourceException(mName);
}


void GloballySharedResource::ptr_base::checkSameLock(const ptr_base &other) const
{
	if (!hasSameLock(other))
		throw std::invalid_argument("different lock object");
}


void GloballySharedResource::ptr_base::swap(ptr_base &other)
{
	GloballySharedResource *temp = this->mLock;
	this->mLock = other.mLock;
	other.mLock = temp;
}


GloballySharedResource::ptr_base &GloballySharedResource::ptr_base::operator=(const ptr_base &other)
	throw(InvalidatedResourceException)
{
	if (!hasSameLock(other)) {
		unref();
		mLock = other.mLock;
		ref();
	}
	return *this;
}

} /* namespace meta */
