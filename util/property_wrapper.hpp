/*
 * PropertyWrapper.hpp
 *
 *  Created on: 10.01.2012
 *      Author: malte
 */

#pragma once
#ifndef UTIL_PROPERTY_WRAPPER_HPP_
#define UTIL_PROPERTY_WRAPPER_HPP_

#include <stdexcept>
#include <boost/exception/all.hpp>

namespace util {

	namespace guards {

		template <class T>
		struct always_true {
			static inline bool check(const T *, const T *) { return true; }
			static inline bool check(const T &, const T &) { return true; }
		};


		template <class T>
		struct always_false {
			static inline bool check(const T *, const T *) { return false; }
			static inline bool check(const T &, const T &) { return false; }
		};


		template <class T, class G1, class G2>
		struct and_g {
			static bool check(const T *oldvalue, const T *newvalue);
			static bool check(const T &oldvalue, const T &newvalue);
		};


		template <class T, class G1, class G2>
		struct or_g {
			static bool check(const T *oldvalue, const T *newvalue);
			static bool check(const T &oldvalue, const T &newvalue);
		};


		template <class T>
		struct old_empty {
			static bool check(const T &oldvalue, const T &);
		};


		template <class T>
		struct new_not_empty {
			static bool check(const T &, const T &newvalue);
		};


		struct old_null_pointer {
			static bool check(const void *oldptr, const void *);
		};


		struct new_not_null_pointer {
			static bool check(const void *, const void *newptr);
		};

	} // namespace guards


	template <class T, class G = guards::always_true<T> >
	class property_wrapper
	{
	protected:
		T m_value;

	public:
		explicit property_wrapper(const T &value);

		property_wrapper();

		inline T &get() { return m_value; }
		inline const T &get() const { return m_value; }

		inline T &operator*() { return get(); }
		inline const T &operator*() const { return get(); }

		inline operator T& () { return get(); }
		inline operator const T& () const { return get(); }

		inline T *operator->() { return &get(); }
		inline const T *operator->() const { return &get(); }

		bool operator==(const T &o) const;

		bool set(const T &new_value);

		property_wrapper<T, G> &operator=(const T &new_value) throw(::std::invalid_argument);

		property_wrapper<T, G> &operator=(const property_wrapper<T, G> &other);
	};


	template <class T, class G = guards::always_true<T> >
	class const_property_wrapper: protected property_wrapper<T, G>
	{
		typedef property_wrapper<T, G> super;

	public:
		explicit const_property_wrapper(const T &value);

		const_property_wrapper();

		inline const T &get() const { return super::get(); }

		inline const T &operator*() const { return get(); }

		inline operator const T& () const { return get(); }

		inline const T *operator->() const { return &get(); }

		bool operator==(const T &o) const;

		bool set(const T &new_value);

		const_property_wrapper<T, G> &operator=(const T &new_value) throw(::std::invalid_argument);

		const_property_wrapper<T, G> &operator=(const property_wrapper<T, G> &other);
	};


	template <class T, class G = guards::always_true<T*> >
	class pointer_property_wrapper: protected property_wrapper<T*, G>
	{
		typedef property_wrapper<T*, G> super;

	public:
		explicit pointer_property_wrapper(T *value);

		pointer_property_wrapper();

		inline T *get() { return super::get(); }
		inline const T *get() const { return super::get(); }

		inline T &operator*() { return *get(); }
		inline const T &operator*() const { return *get(); }

		inline operator T* () { return get(); }
		inline operator const T* () const { return get(); }

		inline T *operator->() { return get(); }
		inline const T *operator->() const { return get(); }

		bool set(T *new_value);

		pointer_property_wrapper<T, G> &operator=(T *new_value) throw(::std::invalid_argument);

		pointer_property_wrapper<T, G> &operator=(const property_wrapper<T*, G> &other);
	};



// implementations ============================================================

	template <typename T, class G>
	inline property_wrapper<T,G>::property_wrapper(const T &value)
		: m_value(value)
	{ }


	template <typename T, class G>
	inline property_wrapper<T,G>::property_wrapper()
	{ }


	template <typename T, class G>
	inline bool property_wrapper<T,G>::operator==(const T &o) const
	{
		return m_value == o.value;
	}


	template <typename T, class G>
	inline bool property_wrapper<T,G>::set(const T &new_value)
	{
		const bool b = G::check(m_value, new_value);
		if (b) {
			m_value = new_value;
		}
		return b;
	}


	template <typename T, class G>
	property_wrapper<T, G> &property_wrapper<T,G>::operator=(const T &new_value) throw(::std::invalid_argument)
	{
		if (!set(new_value))
			BOOST_THROW_EXCEPTION(::std::invalid_argument("new_value"));
		return *this;
	}


	template <typename T, class G>
	inline property_wrapper<T, G> &property_wrapper<T,G>::operator=(const property_wrapper<T, G> &other)
	{
		m_value = other.m_value;
		return *this;
	}


	template <typename T, class G>
	inline const_property_wrapper<T,G>::const_property_wrapper(const T &value)
		: super(value)
	{ }


	template <typename T, class G>
	inline const_property_wrapper<T,G>::const_property_wrapper()
		: super()
	{ }


	template <typename T, class G>
	inline bool const_property_wrapper<T,G>::operator==(const T &o) const
	{
		return super::operator==(o);
	}


	template <typename T, class G>
	inline bool const_property_wrapper<T,G>::set(const T &new_value)
	{
		return super::set(new_value);
	}


	template <typename T, class G>
	inline
	const_property_wrapper<T, G> &
	const_property_wrapper<T,G>::operator=(const T &new_value)
		throw(::std::invalid_argument)
	{
		return super::operator=(new_value);
	}


	template <typename T, class G>
	inline
	const_property_wrapper<T, G> &
	const_property_wrapper<T,G>::operator=(const property_wrapper<T, G> &other)
	{
		return super::operator=(other);
	}


	template <typename T, class G>
	inline pointer_property_wrapper<T,G>::pointer_property_wrapper(T *value)
		: super(value)
	{ }

	template <typename T, class G>
	inline pointer_property_wrapper<T,G>::pointer_property_wrapper()
		: super(0)
	{ }


	template <typename T, class G>
	inline bool pointer_property_wrapper<T,G>::set(T *new_value)
	{
		return super::set(new_value);
	}


	template <typename T, class G>
	inline
	pointer_property_wrapper<T, G> &
	pointer_property_wrapper<T,G>::operator=(T *new_value)
		throw(::std::invalid_argument)
	{
		return super::operator=(new_value);
	}


	template <typename T, class G>
	inline
	pointer_property_wrapper<T, G> &
	pointer_property_wrapper<T,G>::operator=(const property_wrapper<T*, G> &other)
	{
		return super::operator=(other);
	}


	namespace guards {

		template <class T, class G1, class G2>
		inline bool and_g<T,G1,G2>::check(const T *oldvalue, const T *newvalue)
		{
			return G1::check(oldvalue, newvalue) && G2::check(oldvalue, newvalue);
		}

		template <class T, class G1, class G2>
		inline bool and_g<T,G1,G2>::check(const T &oldvalue, const T &newvalue)
		{
			return G1::check(oldvalue, newvalue) && G2::check(oldvalue, newvalue);
		}


		template <class T, class G1, class G2>
		inline bool or_g<T,G1,G2>::check(const T *oldvalue, const T *newvalue)
		{
			return G1::check(oldvalue, newvalue) || G2::check(oldvalue, newvalue);
		}

		template <class T, class G1, class G2>
		inline bool or_g<T,G1,G2>::check(const T &oldvalue, const T &newvalue)
		{
			return G1::check(oldvalue, newvalue) || G2::check(oldvalue, newvalue);
		}


		template <class T>
		inline bool old_empty<T>::check(const T &oldvalue, const T &)
		{
			return oldvalue.empty();
		}


		template <class T>
		inline bool new_not_empty<T>::check(const T &, const T &newvalue)
		{
			return !newvalue.empty();
		}


		inline bool old_null_pointer::check(const void *oldptr, const void *)
		{
			return oldptr == 0;
		}


		inline bool new_not_null_pointer::check(const void *, const void *newptr)
		{
			return newptr != 0;
		}

	} // namespace guards

} // namespace util
#endif // UTIL_PROPERTY_WRAPPER_HPP_
