/*
 * object_wrapper.hpp
 *
 *  Created on: 20.12.2011
 *      Author: malte
 */

#pragma once
#ifndef SENSORS_OBJECTWRAPPER_HPP_
#define SENSORS_OBJECTWRAPPER_HPP_

#include "common.hpp"
#include "util/memory.hpp"
#include <type_traits>


namespace sensors {

using util::shared_ptr;
using util::weak_ptr;

using std::enable_if;
using std::disable_if;
using std::is_void;

class lock;


template <typename T, typename ParentT = void>
class object_wrapper
{
public:
	typedef object_wrapper<T, ParentT> this_type;
	typedef T basic_type;
	typedef T value_type;
	typedef T element_type;
	typedef ParentT parent_type;

	struct parent_comparator {
		static bool equal(const this_type &a, const this_type &b);

		static bool less(const this_type &a, const this_type &b);
	};

	bool empty() const;
	bool operator!() const;

	T *get();
	const T *get() const;

	T *operator->();
	const T *operator->() const;

	T &operator*();
	const T &operator*() const;

	shared_ptr<parent_type> const &parent() const;

	bool has_parent() const;

protected:
	object_wrapper();

	object_wrapper(basic_type *object);

	object_wrapper(basic_type *object, const shared_ptr<parent_type> &parent);

	~object_wrapper();

	basic_type *m_object;

	shared_ptr<parent_type> m_parent;
};


template <typename T, typename ParentT = void>
class object_wrapper_numbered
	: public object_wrapper<T, ParentT>
{
public:
	typedef object_wrapper_numbered<T, ParentT> this_type;
	typedef object_wrapper<T, ParentT> super;
	typedef typename super::basic_type basic_type;
	typedef typename super::parent_type parent_type;

	struct number_comparator {
		static bool equal(const basic_type *a, const basic_type *b);
		static bool equal(const this_type &a, const basic_type *b);
		static bool equal(const this_type &a, const this_type &b);

		static bool less(const basic_type *a, const basic_type *b);
		static bool less(const this_type &a, const basic_type *b);
		static bool less(const this_type &a, const this_type &b);
	};

	bool operator==(const object_wrapper_numbered<T, ParentT> &o) const;

protected:
	object_wrapper_numbered();

	object_wrapper_numbered(basic_type *object);

	object_wrapper_numbered(basic_type *object, const shared_ptr<parent_type> &parent);
};



// implementations ========================================

namespace internal {

template <typename T, typename P>
struct object_wrapper_parent_comparator_impl {
	typedef object_wrapper<T, P> object_wrapper_t;

	static bool equal(const object_wrapper_t &a, const object_wrapper_t &b)
	{
		return (a.has_parent() && b.has_parent()) ?
				*a.parent() == *b.parent() :
				 a.parent() ==  b.parent();
	}

	static bool less(const object_wrapper_t &a, const object_wrapper_t &b)
	{
		return (a.has_parent() && b.has_parent()) ?
				*a.parent()  < *b.parent() :
				 a.parent()  <  b.parent();
	}
};


template <typename T>
struct object_wrapper_parent_comparator_impl<T, void> {
	typedef object_wrapper<T, void> object_wrapper_t;

	static inline bool equal(const object_wrapper_t &a, const object_wrapper_t &b)
	{
		return a.parent() == b.parent();
	}

	static inline bool less(const object_wrapper_t &a, const object_wrapper_t &b)
	{
		return a.parent()  < b.parent();
	}
};

}

template <typename T, typename P>
inline bool object_wrapper<T,P>::parent_comparator::equal(const this_type &a, const this_type &b)
{
	return internal::object_wrapper_parent_comparator_impl<T,P>::equal(a, b);
}


template <typename T, typename P>
inline bool object_wrapper<T,P>::parent_comparator::less(const this_type &a, const this_type &b)
{
	return internal::object_wrapper_parent_comparator_impl<T,P>::less(a, b);
}


template <typename T, typename P>
inline bool object_wrapper<T,P>::empty() const
{
	return !m_object;
}


template <typename T, typename P>
inline bool object_wrapper<T,P>::operator!() const
{
	return empty();
}


template <typename T, typename P>
inline T *object_wrapper<T,P>::get()
{
	return m_object;
}


template <typename T, typename P>
inline const T *object_wrapper<T,P>::get() const
{
	return m_object;
}


template <typename T, typename P>
inline T *object_wrapper<T,P>::operator->()
{
	return get();
}


template <typename T, typename P>
inline const T *object_wrapper<T,P>::operator->() const
{
	return get();
}


template <typename T, typename P>
inline T &object_wrapper<T,P>::operator*()
{
	return *get();
}


template <typename T, typename P>
inline const T &object_wrapper<T,P>::operator*() const
{
	return *get();
}


template <typename T, typename P>
inline const shared_ptr<P> &object_wrapper<T,P>::parent() const
{
	return m_parent;
}


template <typename T, typename P>
inline bool object_wrapper<T,P>::has_parent() const
{
	return static_cast<bool>(m_parent);
}

template <typename T, typename P>
inline object_wrapper<T,P>::object_wrapper()
	: m_object(0)
	, m_parent()
{
}


template <typename T, typename P>
inline object_wrapper<T,P>::object_wrapper(basic_type *object)
	: m_object(object)
{
}


template <typename T, typename P>
inline object_wrapper<T,P>::object_wrapper(basic_type *object, const shared_ptr<parent_type> &parent)
	: m_object(object)
	, m_parent(parent)
{
}


template <typename T, typename P>
inline object_wrapper<T,P>::~object_wrapper()
{
}


template <typename T, typename P>
inline bool object_wrapper_numbered<T,P>::number_comparator::equal(const basic_type *a, const basic_type *b)
{
	return (a && b) ? a->number == b->number : a == b;
}

template <typename T, typename P>
inline bool object_wrapper_numbered<T,P>::number_comparator::equal(const this_type &a, const basic_type *b)
{
	return equal(a.get(), b);
}

template <typename T, typename P>
inline bool object_wrapper_numbered<T,P>::number_comparator::equal(const this_type &a, const this_type &b)
{
	return equal(a.get(), b.get());
}


template <typename T, typename P>
inline bool object_wrapper_numbered<T,P>::number_comparator::less(const basic_type *a, const basic_type *b)
{
	return (a && b) ? a->number < b->number : a < b;
}

template <typename T, typename P>
inline bool object_wrapper_numbered<T,P>::number_comparator::less(const this_type &a, const basic_type *b)
{
	return less(a.get(), b);
}

template <typename T, typename P>
inline bool object_wrapper_numbered<T,P>::number_comparator::less(const this_type &a, const this_type &b)
{
	return less(a.get(), b.get());
}


template <typename T, typename P>
inline bool object_wrapper_numbered<T,P>::operator==(const object_wrapper_numbered<T,P> &o) const
{
	return number_comparator::equal(*this, o) &&
			super::parent_comparator::equal(*this, o);
}


template <typename T, typename P>
inline object_wrapper_numbered<T,P>::object_wrapper_numbered()
	: super()
{
}


template <typename T, typename P>
inline object_wrapper_numbered<T,P>::object_wrapper_numbered(basic_type *object)
	: super(object)
{
}


template <typename T, typename P>
inline object_wrapper_numbered<T,P>::object_wrapper_numbered(basic_type *object, const shared_ptr<parent_type> &parent)
	: super(object, parent)
{
}

} /* namespace sensors */
#endif /* SENSORS_OBJECTWRAPPER_HPP_ */
