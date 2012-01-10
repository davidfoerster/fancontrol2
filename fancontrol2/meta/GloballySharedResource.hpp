/*
 * GloballySharedResource.hpp
 *
 *  Created on: 16.12.2011
 *      Author: malte
 */

#pragma once
#ifndef GLOBALLYSHAREDRESOURCE_HPP_
#define GLOBALLYSHAREDRESOURCE_HPP_

#include <boost/format.hpp>
#include <string>
#include <stdexcept>

namespace meta {


class InvalidatedResourceException: public std::logic_error
{
	typedef std::string string;

public:
	explicit InvalidatedResourceException(const string &resourceName) throw();

	explicit InvalidatedResourceException(const void *resource) throw();

	virtual ~InvalidatedResourceException() throw() {};

private:
	static string makeDescription(const string &resourceName);

	static string makeDescription(const void *resource);

	static const char *descriptionTemplate();
};


class GloballySharedResource
{
	typedef std::string string;

private:
	class ptr_base {
	public:
		ptr_base(const ptr_base &other) : mLock(other.mLock) { ref(); }

		virtual ~ptr_base() { unref(); }

		// const GloballySharedResource &getLock() const { return mLock; }

		bool hasSameLock(const ptr_base &other) const { return this->mLock == other.mLock; }

		bool operator==(const ptr_base &other) { return hasSameLock(other); }

		bool operator!=(const ptr_base &other) { return !hasSameLock(other); }

		void swap(ptr_base &other);

		ptr_base &operator=(const ptr_base &other) throw(InvalidatedResourceException);

	protected:
		explicit ptr_base(GloballySharedResource *lock) : mLock(lock) { ref(); }

		void checkInvalid() const throw(InvalidatedResourceException) { if (mLock) mLock->checkInvalid(); }

		void checkSameLock(const ptr_base &other) const;

	private:
		void ref() { if (mLock) mLock->ref(); }

		void unref() { if (mLock) mLock->unref(); }

		GloballySharedResource *mLock;
	};

public:
	template <typename T>
	class ptr: ptr_base {
	public:
		explicit ptr(T *p) : ptr_base(NULL), p(p) {}

		ptr(const ptr<T> &other) : ptr_base(other), p(other.p) {}

		T *getUnprotectedPtr() { return p; }
		const T *getUnprotectedPtr() const { return p; }

		T *get() throw(InvalidatedResourceException) { checkInvalid(); return p; }
		const T *get() const throw(InvalidatedResourceException) { checkInvalid(); return p; }

		T *operator->() throw(InvalidatedResourceException) { return get(); }
		const T *operator->() const throw(InvalidatedResourceException) { return get(); }

		T &operator*() throw(InvalidatedResourceException) { return *get(); }
		const T &operator*() const throw(InvalidatedResourceException) { return *get(); }

		void swap(ptr<T> &other) {
			ptr_base::swap(other);
			T *temp = this->p;
			this->p = other.p;
			other.p = temp;
		}

		ptr<T> &operator=(const ptr<T> &other) throw(InvalidatedResourceException) {
			ptr_base::operator=(other);
			this->p = other.p;
			return *this;
		}

		bool operator==(const ptr<T> &other) { return this->p == other.p && hasSameLock(other); }

		bool operator!=(const ptr<T> &other) { return !this->operator==(other); }

	protected:
		T *p;

	private:
		ptr(GloballySharedResource *lock, T *p) : ptr_base(lock), p(p) {}

		friend class GloballySharedResource;
	};

	GloballySharedResource(const char *name = NULL, bool autoRelease = false);

	GloballySharedResource(const string &name, bool autoRelease = false);

	virtual ~GloballySharedResource();

	size_t getReferenceCount() const { return mReferenceCount; }

	bool isValid() const { return !mInvalid; }

	void invalidate() { mInvalid = true; }

	const string &getName() const { return mName; }

	bool doesAutoRelease() const { return mAutoRelease; }

	void setAutoRelease(bool autoRelease);

	template <typename T>
	ptr<T> makePtr(T *p) { return ptr<T>(this, p); }

protected:
	virtual void release() = 0;

	virtual bool isReadyForRelease();

private:
	string mName;

	size_t mReferenceCount;

	bool mInvalid, mAutoRelease;

	void ref() throw(InvalidatedResourceException);

	void unref();

	void release_internal();

	string makeDefaultName();

	void checkInvalid() const throw(InvalidatedResourceException);

	friend class ptr_base;
};

} /* namespace meta */
#endif /* GLOBALLYSHAREDRESOURCE_HPP_ */
