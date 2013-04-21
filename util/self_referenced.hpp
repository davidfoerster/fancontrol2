/*
 * self_referenced.hpp
 *
 *  Created on: 28.01.2012
 *      Author: malte
 */

#pragma once
#ifndef UTIL_SELF_REFERENCED_HPP_
#define UTIL_SELF_REFERENCED_HPP_

#include "logging.hpp"
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/assert.hpp>


namespace util {

	using ::boost::weak_ptr;
	using ::boost::shared_ptr;
	using ::boost::make_shared;


	template <class T>
	class self_referenced
	{
	public:
		typedef self_referenced<T> selfreference_type;

		template <typename... Args>
		static shared_ptr<T> make(Args... args);

		self_referenced<T> &operator=(const self_referenced<T> &);

		shared_ptr<T> selfreference();
		shared_ptr<const T> selfreference() const;

	protected:
		struct factory_tag_t;
		typedef const factory_tag_t* factory_tag;

		explicit self_referenced(const factory_tag);

		self_referenced(const self_referenced<T> &);

		~self_referenced();

	private:
		void init();

		self_referenced();

		void selfreference(const shared_ptr<T> &);

		weak_ptr<T> m_selfreference;
	};


	namespace internal {

		void _self_reference_warning();

	}

// implementation =============================================================

	template <class T>
	inline self_referenced<T>::self_referenced(const factory_tag)
	{
		init();
	}


	template <class T>
	inline void self_referenced<T>::init()
	{
		/*
		const self_referenced<T> *p = NULL;  // this must be a base type of T
		UTIL_UNUSED(p);
		*/
	}


	template <class T>
	inline self_referenced<T>::~self_referenced()
	{
	}


	template <class T>
	inline self_referenced<T>::self_referenced(const self_referenced<T> &)
	{
		// don't copy the self-reference
	}


	template <class T>
	inline self_referenced<T> &self_referenced<T>::operator=(const self_referenced<T> &)
	{
		// don't copy the self-reference
	}


	template <class T>
	inline void self_referenced<T>::selfreference(const shared_ptr<T> &ptr)
	{
		BOOST_ASSERT(dynamic_cast<const self_referenced<T>*>(ptr.get()) == this);
		m_selfreference = ptr;
	}


	template <class T>
	inline shared_ptr<T> self_referenced<T>::selfreference()
	{
		return m_selfreference.lock();
	}


	template <class T>
	inline shared_ptr<const T> self_referenced<T>::selfreference() const
	{
		return m_selfreference.lock();
	}


	template <class T>
	template <typename... Args>
	inline shared_ptr<T> self_referenced<T>::make(Args... args)
	{
		shared_ptr<T> ptr(make_shared<T>(args..., factory_tag()));
		ptr->self_referenced<T>::selfreference(ptr);
		return ptr;
	}

} /* namespace util */
#endif // UTIL_SELF_REFERENCED_HPP_
