/*
 * conditional_shared_ptr.hpp
 *
 *  Created on: 19.01.2012
 *      Author: malte
 */

#pragma once
#ifndef SENSORS_CONDITIONAL_SHARED_PTR_HPP_
#define SENSORS_CONDITIONAL_SHARED_PTR_HPP_

#if defined(BOOST_DISABLE_THREADS) || defined(BOOST_SP_DISABLE_THREADS)
#include <atomic>
#endif
#include <memory>
#include <cstddef>
#include <boost/assert.hpp>


namespace sensors {

	namespace internal {

		class shared_reference_counter_base
		{
		public:
			typedef int counter_t;

			virtual ~shared_reference_counter_base() {};

			void ref();

			bool unref();

		protected:
			shared_reference_counter_base();

		private:
		#if defined(BOOST_DISABLE_THREADS) || defined(BOOST_SP_DISABLE_THREADS)
			counter_t
		#else
			std::atomic<counter_t>
		#endif
					m_counter;
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

			bool unref();

			shared_reference_counter_base *m_reference_counter;
		};

	} // namespace internal


	template <typename T>
	class conditional_shared_ptr: public internal::conditional_shared_ptr_base
	{
	public:
		typedef const T element_type;
		typedef const T value_type;

		conditional_shared_ptr();

		template <typename Y>
		conditional_shared_ptr(const Y* ptr);

		template <typename Y>
		conditional_shared_ptr(std::unique_ptr<Y> &ptr);

		template <typename Y, class D>
		conditional_shared_ptr(std::unique_ptr<Y> &ptr, D deleter);

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
		: internal::conditional_shared_ptr_base(nullptr)
		, m_ptr(nullptr)
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
	conditional_shared_ptr<T>::conditional_shared_ptr(std::unique_ptr<Y> &ptr)
		: internal::conditional_shared_ptr_base(ptr.get())
		, m_ptr(ptr.get())
	{
		ptr.release();
	}


	template <typename T>
	template <typename Y, class D>
	conditional_shared_ptr<T>::conditional_shared_ptr(std::unique_ptr<Y> &ptr, D deleter)
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
			BOOST_ASSERT(m_counter > 0);
		#if defined(BOOST_DISABLE_THREADS) || defined(BOOST_SP_DISABLE_THREADS)
			++m_counter;
		#else
			m_counter.fetch_add(1);
		#endif
		}


		inline
		bool shared_reference_counter_base::unref()
		{
			BOOST_ASSERT(m_counter > 0);
		#if defined(BOOST_DISABLE_THREADS) || defined(BOOST_SP_DISABLE_THREADS)
			return --m_counter != 0;
		#else
			return m_counter.fetch_sub(1) != 1;
		#endif
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
		conditional_shared_ptr_base::conditional_shared_ptr_base(T *ptr __attribute__((unused)))
			: m_reference_counter(nullptr)
		{
		}


		template <typename T, class D>
		conditional_shared_ptr_base::conditional_shared_ptr_base(T *ptr, D deleter)
			: m_reference_counter(ptr ? new shared_reference_counter_impl_deleter<T,D>(ptr, deleter) : nullptr)
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
			if (!unref())
				delete m_reference_counter;
		}


		inline
		void conditional_shared_ptr_base::ref()
		{
			if (m_reference_counter)
				m_reference_counter->ref();
		}


		inline
		bool conditional_shared_ptr_base::unref()
		{
			return !m_reference_counter ||
				m_reference_counter->unref();
		}

	} // namespace internal

} /* namespace sensors */
#endif // SENSORS_CONDITIONAL_SHARED_PTR_HPP_
