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
			static inline bool check(const T *oldvalue, const T *newvalue) {
				return G1::check(oldvalue, newvalue) && G2::check(oldvalue, newvalue);
			}

			static inline bool check(const T &oldvalue, const T &newvalue) {
				return G1::check(oldvalue, newvalue) && G2::check(oldvalue, newvalue);
			}
		};


		template <class T, class G1, class G2>
		struct or_g {
			static inline bool check(const T *oldvalue, const T *newvalue) {
				return G1::check(oldvalue, newvalue) || G2::check(oldvalue, newvalue);
			}

			static inline bool check(const T &oldvalue, const T &newvalue) {
				return G1::check(oldvalue, newvalue) || G2::check(oldvalue, newvalue);
			}
		};


		template <class T>
		struct old_empty {
			static inline bool check(const T &oldvalue, const T &) { return oldvalue.empty(); }
		};


		template <class T>
		struct new_not_empty {
			static inline bool check(const T &, const T &newvalue) { return !newvalue.empty(); }
		};


		struct old_null_pointer {
			static inline bool check(const void *oldptr, const void *) { return oldptr == 0; }
		};


		struct new_not_null_pointer {
			static inline bool check(const void *, const void *newptr) { return newptr != 0; }
		};

	}


	template <class T, class G = guards::always_true<T> >
	class property_wrapper
	{
	protected:
		T m_value;

	public:
		explicit property_wrapper(const T &value) : m_value(value) {}

		property_wrapper() {}

		inline T &get() { return m_value; }
		inline const T &get() const { return m_value; }

		inline T &operator*() { return get(); }
		inline const T &operator*() const { return get(); }

		inline operator T& () { return get(); }
		inline operator const T& () const { return get(); }

		inline T *operator->() { return &get(); }
		inline const T *operator->() const { return &get(); }

		inline bool operator==(const T &o) const { return m_value == o.value; }

		bool set(const T &new_value);

		property_wrapper<T, G> &operator=(const T &new_value) throw(::std::invalid_argument);

		property_wrapper<T, G> &operator=(const property_wrapper<T, G> &other);
	};


	template <class T, class G = guards::always_true<T> >
	class const_property_wrapper: protected property_wrapper<T, G>
	{
		typedef property_wrapper<T, G> super;

	public:
		explicit const_property_wrapper(const T &value) : super(value) {}

		const_property_wrapper() : super() {}

		inline const T &get() const { return super::get(); }

		inline const T &operator*() const { return get(); }

		inline operator const T& () const { return get(); }

		inline const T *operator->() const { return &get(); }

		inline bool operator==(const T &o) const { return super::operator==(o); }

		inline bool set(const T &new_value) { return super::set(new_value); }

		inline const_property_wrapper<T, G> &operator=(const T &new_value) throw(::std::invalid_argument) {
			return super::operator=(new_value);
		}

		inline const_property_wrapper<T, G> &operator=(const property_wrapper<T, G> &other) {
			return super::operator=(other);
		}
	};


	template <class T, class G = guards::always_true<T*> >
	class pointer_property_wrapper: protected property_wrapper<T*, G>
	{
		typedef property_wrapper<T*, G> super;

	public:
		explicit pointer_property_wrapper(T *value) : super(value) {}

		pointer_property_wrapper() : super(0) {}

		inline T *get() { return super::get(); }
		inline const T *get() const { return super::get(); }

		inline T &operator*() { return *get(); }
		inline const T &operator*() const { return *get(); }

		inline operator T* () { return get(); }
		inline operator const T* () const { return get(); }

		inline T *operator->() { return get(); }
		inline const T *operator->() const { return get(); }

		inline bool set(T *new_value) { return super::set(new_value); }

		inline pointer_property_wrapper<T, G> &operator=(T *new_value) throw(::std::invalid_argument) {
			return super::operator=(new_value);
		}

		inline pointer_property_wrapper<T, G> &operator=(const property_wrapper<T*, G> &other) {
			return super::operator=(other);
		}
	};



// implementations ============================================================

	template <typename T, class G>
	bool property_wrapper<T,G>::set(const T &new_value)
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
	property_wrapper<T, G> &property_wrapper<T,G>::operator=(const property_wrapper<T, G> &other)
	{
		m_value = other.m_value;
		return *this;
	}

}

#endif // UTIL_PROPERTY_WRAPPER_HPP_
