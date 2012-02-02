/*
 * conditional_shared_ptr.hpp
 *
 *  Created on: 19.01.2012
 *      Author: malte
 */

#pragma once
#ifndef CONDITIONAL_SHARED_PTR_HPP_
#define CONDITIONAL_SHARED_PTR_HPP_

#ifndef BOOST_SP_DISABLE_THREADS
#	warning The locking mechanism of conditional_shared_ptr is not thread-safe!
#endif


#include <cstddef>
#include <boost/assert.hpp>

namespace std {
	template <class> class auto_ptr;
}

namespace sensors {

namespace internal {

	class shared_reference_counter_base
	{
	public:
		typedef std::size_t size_type;

		virtual ~shared_reference_counter_base() {};

		bool unique() const;

		void ref();

		void unref();

	protected:
		shared_reference_counter_base();

	private:
		size_type m_counter;
	};


	template <typename T>
	class shared_reference_counter_impl: public shared_reference_counter_base
	{
	public:
		shared_reference_counter_impl(T *ptr);

		virtual ~shared_reference_counter_impl();

	private:
		T *m_ptr;
	};


	template <typename T, class D>
	class shared_reference_counter_impl_deleter: public shared_reference_counter_base
	{
	public:
		shared_reference_counter_impl_deleter(T *ptr, D deleter);

		virtual ~shared_reference_counter_impl_deleter();

	private:
		T *m_ptr;

		D m_deleter;
	};


	class conditional_shared_ptr_base
	{
	public:
		bool unique() const;

		conditional_shared_ptr_base(const conditional_shared_ptr_base &o);

	protected:
		template <typename T>
		conditional_shared_ptr_base(T *ptr);

		template <typename T, class D>
		conditional_shared_ptr_base(T *ptr, D deleter);

		~conditional_shared_ptr_base();

		conditional_shared_ptr_base &operator=(const conditional_shared_ptr_base &o);

	private:
		void ref();

		void unref();

		shared_reference_counter_base *m_reference_counter;
	};
}


template <typename T>
class conditional_shared_ptr: public internal::conditional_shared_ptr_base
{
public:
	typedef const T element_type;
	typedef const T value_type;

	//typedef typename boost::remove_const<T>::type wrapped_type_no_const;

	conditional_shared_ptr();

	template <typename Y>
	conditional_shared_ptr(const Y* ptr);

	template <typename Y>
	conditional_shared_ptr(std::auto_ptr<Y> &ptr);

	template <typename Y, class D>
	conditional_shared_ptr(std::auto_ptr<Y> &ptr, D deleter);

	value_type *get() const;

	value_type *operator->() const;

	value_type &operator*() const;

	bool operator!() const;

	bool operator==(const T *ptr) const;
	bool operator==(const conditional_shared_ptr<T> &o);

	bool operator<(const T *ptr) const;
	bool operator<(const conditional_shared_ptr<T> &o);

private:
	value_type *m_ptr;
};



// implementations ========================================

template <typename T>
conditional_shared_ptr<T>::conditional_shared_ptr()
	: internal::conditional_shared_ptr_base(static_cast<T*>(0))
	, m_ptr(0)
{
}


template <typename T>
template <typename Y>
conditional_shared_ptr<T>::conditional_shared_ptr(const Y *ptr)
	: internal::conditional_shared_ptr_base(ptr)
	, m_ptr(ptr)
{
}


template <typename T>
template <typename Y>
conditional_shared_ptr<T>::conditional_shared_ptr(std::auto_ptr<Y> &ptr)
	: internal::conditional_shared_ptr_base(ptr.get())
	, m_ptr(ptr.get())
{
	ptr.release();
}


template <typename T>
template <typename Y, class D>
conditional_shared_ptr<T>::conditional_shared_ptr(std::auto_ptr<Y> &ptr, D deleter)
	: internal::conditional_shared_ptr_base(ptr.get(), deleter)
	, m_ptr(ptr.get())
{
	ptr.release();
}


template <typename T>
const T *conditional_shared_ptr<T>::get() const
{
	return m_ptr;
}


template <typename T>
const T *conditional_shared_ptr<T>::operator->() const
{
	return get();
}


template <typename T>
const T &conditional_shared_ptr<T>::operator*() const
{
	return *get();
}


template <typename T>
bool conditional_shared_ptr<T>::operator!() const
{
	return !m_ptr;
}


template <typename T>
bool conditional_shared_ptr<T>::operator==(const T *ptr) const
{
	return m_ptr == ptr;
}

template <typename T>
bool conditional_shared_ptr<T>::operator==(const conditional_shared_ptr<T> &o)
{
	return m_ptr == o.get();
}


template <typename T>
bool conditional_shared_ptr<T>::operator<(const T *ptr) const
{
	return m_ptr < ptr;
}

template <typename T>
bool conditional_shared_ptr<T>::operator<(const conditional_shared_ptr<T> &o)
{
	return m_ptr < o.get();
}


namespace internal {

inline
shared_reference_counter_base::shared_reference_counter_base()
	: m_counter(1)
{
}


inline
void shared_reference_counter_base::ref()
{
	m_counter += 1;
}


inline
void shared_reference_counter_base::unref()
{
	BOOST_ASSERT(!unique());
	m_counter -= 1;
}


inline
bool shared_reference_counter_base::unique() const
{
	return m_counter == 1;
}


template <typename T>
shared_reference_counter_impl<T>::shared_reference_counter_impl(T *ptr)
	: m_ptr(ptr)
{
}


template <typename T>
shared_reference_counter_impl<T>::~shared_reference_counter_impl()
{
	if (m_ptr)
		delete m_ptr;
}


template <typename T, class D>
shared_reference_counter_impl_deleter<T,D>::shared_reference_counter_impl_deleter(T *ptr, D deleter)
	: m_ptr(ptr)
	, m_deleter(deleter)
{
}


template <typename T, class D>
shared_reference_counter_impl_deleter<T,D>::~shared_reference_counter_impl_deleter()
{
	if (m_ptr)
		m_deleter(m_ptr);
}


template <typename T>
conditional_shared_ptr_base::conditional_shared_ptr_base(T *ptr)
	: m_reference_counter(0)
{
}


template <typename T, class D>
conditional_shared_ptr_base::conditional_shared_ptr_base(T *ptr, D deleter)
	: m_reference_counter(ptr ? new shared_reference_counter_impl_deleter<T,D>(ptr, deleter) : 0)
{
}


inline
conditional_shared_ptr_base::conditional_shared_ptr_base(const conditional_shared_ptr_base &o)
	: m_reference_counter(o.m_reference_counter)
{
	ref();
}


inline
conditional_shared_ptr_base &conditional_shared_ptr_base::operator=(const conditional_shared_ptr_base &o)
{
	if (m_reference_counter != o.m_reference_counter) {
		unref();
		m_reference_counter = o.m_reference_counter;
		ref();
	}
	return *this;
}


inline
conditional_shared_ptr_base::~conditional_shared_ptr_base()
{
	if (unique())
		delete m_reference_counter;
}


inline
void conditional_shared_ptr_base::ref()
{
	if (m_reference_counter)
		m_reference_counter->ref();
}


inline
void conditional_shared_ptr_base::unref()
{
	if (m_reference_counter)
		m_reference_counter->unref();
}


inline
bool conditional_shared_ptr_base::unique() const
{
	return m_reference_counter && m_reference_counter->unique();
}

}

} /* namespace sensors */
#endif /* CONDITIONAL_SHARED_PTR_HPP_ */
