/*
 * self_referenced.hpp
 *
 *  Created on: 28.01.2012
 *      Author: malte
 */

#pragma once
#ifndef SELF_REFERENCED_HPP_
#define SELF_REFERENCED_HPP_

#include "meta/utility.hpp"

#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <boost/assert.hpp>

namespace meta {

using boost::weak_ptr;
using boost::shared_ptr;
using boost::make_shared;


template <class T>
class self_referenced
{
public:
	typedef meta::self_referenced<T> selfreference_type;

	static shared_ptr<T> make();

	template <typename A1>
	static shared_ptr<T> make(const A1&);

	template <typename A1, typename A2>
	static shared_ptr<T> make(const A1&, const A2&);

	template <typename A1, typename A2, typename A3>
	static shared_ptr<T> make(const A1&, const A2&, const A3&);

	template <typename A1, typename A2, typename A3, typename A4>
	static shared_ptr<T> make(const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4);

	struct factory_tag { };

	self_referenced<T> &operator=(const self_referenced<T> &);

	shared_ptr<T> selfreference();
	shared_ptr<const T> selfreference() const;

protected:
	template <class Tag>
	explicit self_referenced(Tag);

	explicit self_referenced(factory_tag);

	self_referenced(const self_referenced<T> &);

	~self_referenced();

private:
	void init();

	void selfreference(const shared_ptr<T> &);

	weak_ptr<T> m_selfreference;
};


namespace internal {

void _self_reference_warning();

}

// implementation =============================================================

template <class T>
template <class Tag>
self_referenced<T>::self_referenced(Tag)
{
#ifndef NDEBUG
	internal::_self_reference_warning();
#endif
	init();
}


template <class T>
self_referenced<T>::self_referenced(factory_tag)
{
	init();
}


template <class T>
void self_referenced<T>::init()
{
	const self_referenced<T> *p = static_cast<T*>(0);  // this must be a base type of T
	META_UNUSED(p);
}


template <class T>
self_referenced<T>::~self_referenced()
{
}


template <class T>
self_referenced<T>::self_referenced(const self_referenced<T> &)
{
	// don't copy the self-reference
}


template <class T>
self_referenced<T> &self_referenced<T>::operator=(const self_referenced<T> &)
{
	// don't copy the self-reference
}


template <class T>
void self_referenced<T>::selfreference(const shared_ptr<T> &ptr)
{
	BOOST_ASSERT(dynamic_cast<self_referenced<T>*>(ptr.get()) == this);
	m_selfreference = ptr;
}


template <class T>
shared_ptr<T> self_referenced<T>::selfreference()
{
	return m_selfreference.lock();
}


template <class T>
shared_ptr<const T> self_referenced<T>::selfreference() const
{
	return m_selfreference.lock();
}


template <class T>
shared_ptr<T> self_referenced<T>::make()
{
	shared_ptr<T> ptr(make_shared<T>(factory_tag()));
	ptr->self_referenced<T>::selfreference(ptr);
	return ptr;
}


template <class T>
template <typename A1>
shared_ptr<T> self_referenced<T>::make(const A1 &a1)
{
	shared_ptr<T> ptr(make_shared<T>(a1, factory_tag()));
	ptr->self_referenced<T>::selfreference(ptr);
	return ptr;
}


template <class T>
template <typename A1, typename A2>
shared_ptr<T> self_referenced<T>::make(const A1 &a1, const A2 &a2)
{
	shared_ptr<T> ptr(make_shared<T>(a1, a2, factory_tag()));
	ptr->self_referenced<T>::selfreference(ptr);
	return ptr;
}


template <class T>
template <typename A1, typename A2, typename A3>
shared_ptr<T> self_referenced<T>::make(const A1 &a1, const A2 &a2, const A3 &a3)
{
	shared_ptr<T> ptr(make_shared<T>(a1, a2, a3, factory_tag()));
	ptr->self_referenced<T>::selfreference(ptr);
	return ptr;
}


template <class T>
template <typename A1, typename A2, typename A3, typename A4>
shared_ptr<T> self_referenced<T>::make(const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
	shared_ptr<T> ptr(make_shared<T>(a1, a2, a3, a4, factory_tag()));
	ptr->self_referenced<T>::selfreference(ptr);
	return ptr;
}

} /* namespace meta */
#endif /* SELF_REFERENCED_HPP_ */
