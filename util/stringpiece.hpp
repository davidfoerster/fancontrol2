/*
 * stringpiece.hpp
 *
 *  Created on: 24.09.2012
 *      Author: malte
 */

#pragma once
#ifndef UTIL_STRINGPIECE_HPP_
#define UTIL_STRINGPIECE_HPP_

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/size.hpp>
#include <boost/range/iterator.hpp>
#include <boost/range/value_type.hpp>
#include <boost/range/category.hpp>
#include <boost/range/algorithm/find.hpp>
#include <boost/range/algorithm/equal.hpp>
#include <boost/range/algorithm/mismatch.hpp>
#include <boost/range/algorithm/lexicographical_compare.hpp>
#include <boost/type_traits/make_unsigned.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/utility/enable_if.hpp>
#include <algorithm>
#include <iterator>
#include <utility>
#include <string>
#include <ostream>
#include <stdexcept>
#include "algorithm.hpp"


namespace util {

	namespace detail {

		template <typename Range>
		static inline
		typename ::boost::disable_if<
			::boost::is_same<
				typename ::boost::range_category<Range>::type,
				::std::random_access_iterator_tag
			>,
			bool
		>::type
		is_c_str(const Range &r) {
			return false;
		}

		template <typename Range>
		static inline
		typename ::boost::enable_if<
			::boost::is_same<
			 typename ::boost::range_category<Range>::type,
				::std::random_access_iterator_tag
			>,
			bool
		>::type
		is_c_str(const Range &r) {
			const typename ::boost::range_const_iterator<Range>::type begin = ::boost::begin(r);
			const typename ::boost::range_difference<Range>::type size = ::boost::size(r);
			const typename ::boost::range_value<Range>::type *const data_end = (&*begin) + size;
			return data_end == &*(begin + size) && *data_end == 0;
		}

	}


	template <typename Iterator>
	class basic_stringpiece
		: public ::std::pair<Iterator, Iterator>
	{
	private:
		typedef ::std::pair<Iterator, Iterator> base_t;
		typedef ::std::iterator_traits<Iterator> iterator_traits;

	public:
		typedef basic_stringpiece<Iterator> self_t;
		typedef Iterator iterator;
		typedef typename ::boost::range_const_iterator<base_t>::type const_iterator;
		typedef typename iterator_traits::value_type value_type;
		typedef typename iterator_traits::pointer pointer;
		typedef typename iterator_traits::reference reference;
		typedef typename iterator_traits::difference_type difference_type;
		typedef typename ::boost::make_unsigned<difference_type>::type size_type;

		struct char_traits
			: public ::std::char_traits<value_type>
		{
			static inline bool le(const value_type &a, const value_type &b) { return lt(a, b) || eq(a, b); }
			static inline bool gt(const value_type &a, const value_type &b) { return !le(a, b); }
			static inline bool ge(const value_type &a, const value_type &b) { return !lt(a, b); }
		};


		static const basic_stringpiece<Iterator> &empty_string() {
			static const basic_stringpiece<Iterator> &e = *new basic_stringpiece<Iterator>();
			return e;
		}


		inline basic_stringpiece() { }

		inline basic_stringpiece(const Iterator &begin, const Iterator &end)
			: base_t(begin, end)
		{ }

		inline basic_stringpiece(const ::std::pair<Iterator, Iterator> &other)
			: base_t(other)
		{ }

		template <typename U>
		inline explicit basic_stringpiece(const ::std::pair<U, U> &other)
			: base_t(other)
		{ }

		template <class Allocator>
		inline basic_stringpiece(const ::std::basic_string<value_type, Allocator> &other)
			: base_t( (other.c_str(), base_t(other.data(), other.data()+other.size())) )
		{ }

		inline explicit basic_stringpiece(const value_type *s)
			: base_t(s, s ? s + char_traits::length(s) : nullptr)
		{ }

		inline basic_stringpiece(const value_type *s, size_type length)
			: base_t(s, s + length)
		{ }

		template <size_type Size>
		inline basic_stringpiece(const value_type (&s)[Size])
			: base_t(s, s + Size - !s[Size-1])
		{ }

		virtual ~basic_stringpiece() { }

		inline size_type size() const { return end() - begin(); }
		inline size_type length() const { return size(); }
		inline bool empty() const { return begin() == end(); }
		inline bool operator!() const { return empty(); }

		inline
		typename ::boost::enable_if<
			::boost::is_same<
				typename iterator_traits::iterator_category,
				::std::random_access_iterator_tag
			>,
			const value_type*
		>::type
		c_str() const {
			if (detail::is_c_str(*this))
				return &*begin();
			throw ::std::logic_error("cannot convert to C string");
		}

		template <class Alloc>
		inline ::std::basic_string<value_type, Alloc> &str(::std::basic_string<value_type, Alloc> &s) const {
			if (!empty()) {
				s.assign(begin(), end());
			} else {
				s.clear();
			}
			return s;
		}

		template <class Alloc>
		inline ::std::basic_string<value_type, Alloc> str() const {
			return !empty() ?
				::std::basic_string<value_type, Alloc>(begin(), end()) :
				::std::basic_string<value_type, Alloc>();
		}

		inline ::std::basic_string<value_type> str() const {
			return !empty() ?
				::std::basic_string<value_type>(begin(), end()) :
				::std::basic_string<value_type>();
		}

		inline Iterator begin() const { return this->first; }
		inline Iterator end() const { return this->second; }

		inline const value_type &front() const { return *begin(); }
		inline const value_type &back() const { return *(!empty() ? end()-1 : this->end()); }

		const value_type &operator[](size_type i) const { return begin()[i]; }

		self_t substr(size_type pos) const { return self_t(begin() + pos, end()); }

		self_t substr(size_type pos, size_type n) const {
			const Iterator begin = this->begin() + pos;
			return self_t(begin, begin + n);
		}

		inline void clear() { this->second = this->first = iterator(); }

		template <typename Range>
		bool operator==(const Range &r) const { return ::boost::equal(*this, r, char_traits::eq); }

		inline bool operator==(value_type *o) const {
			return operator==(const_cast<const value_type*>(o));
		}

		bool operator==(const value_type *o) const {
			iterator s = begin();

			if (o == &*s) {
				return true;
			} else if (o) {
				for ( ; s != end() && *o; ++s, ++o) {
					if (!char_traits::eq(*s, *o))
						return false;
				}
				return s == end() && !*o;
			} else {
				return false;
			}
		}

		template <typename Range>
		const bool operator!=(const Range &r) const { return !operator==(r); }

		template <typename Range>
		bool operator<(const Range &r) const {
			return ::boost::lexicographical_compare(*this, r, &char_traits::lt);
		}

		template <typename Range>
		bool operator<=(const Range &r) const {
			return ::boost::lexicographical_compare(*this, r, &char_traits::le);
		}

		template <typename Range>
		bool operator>(const Range &r) const {
			return ::boost::lexicographical_compare(*this, r, &char_traits::gt);
		}

		template <typename Range>
		bool operator>=(const Range &r) const {
			return ::boost::lexicographical_compare(*this, r, &char_traits::ge);
		}

		template <typename Range>
		int compare(const Range &r) const {
			const ::std::pair<Iterator, typename ::boost::range_const_iterator<Range>::type>
				mismatch = ::boost::mismatch(*this, r);
			const value_type
				c1 = (mismatch.first != this->end()) ? *mismatch.first : static_cast<value_type>(0),
				c2 = (mismatch.second != ::boost::end(r)) ? *mismatch.second : static_cast<value_type>(0);
			return util::compare(c1, c2, &char_traits::lt);
		}

		bool starts_with(const value_type *prefix) const {
			Iterator first = begin(), last = end();
			for ( ; first != last && *prefix != 0; ++first, ++prefix) {
				if (*first != *prefix)
					return false;
			}
			return first != last || *prefix == 0;
		}

		bool prefix_of(const value_type *s) const {
			Iterator first = begin(), last = end();
			for ( ; first != last && *s != 0; ++first, ++s) {
				if (*first != *s)
					return false;
			}
			return first == last;
		}

	};


	template <typename CharT = char, class Alloc = ::std::allocator<CharT> >
	class basic_stringpiece_alloc
		: public basic_stringpiece<const CharT*>
		, private ::std::allocator<CharT>
	{
		typedef basic_stringpiece<const CharT*> sp_t;

	public:
		typedef basic_stringpiece_alloc<CharT, Alloc> self_t;
		typedef Alloc allocator_type;
		typedef typename sp_t::iterator iterator;
		typedef typename sp_t::value_type value_type;
		typedef typename sp_t::difference_type difference_type;
		typedef typename sp_t::size_type size_type;
		typedef typename sp_t::char_traits char_traits;


		inline explicit basic_stringpiece_alloc(const Alloc &alloc = Alloc())
			: sp_t(nullptr, nullptr)
			, Alloc(alloc)
			, m_capacity(0)
		{ }

		inline basic_stringpiece_alloc(const self_t &other)
			: Alloc(other)
		{
			init(other, !!other.m_capacity);
		}

		template <class A>
		inline basic_stringpiece_alloc(const basic_stringpiece_alloc<CharT, A> &other, const Alloc &alloc = Alloc())
			: Alloc(alloc)
		{
			init(other, !!other.m_capcacity);
		}

		template <typename Range>
		inline explicit basic_stringpiece_alloc(const Range &r, bool allocate_always = true, const Alloc &alloc = Alloc())
			: Alloc(alloc)
		{
			init(r, allocate_always);
		}

		virtual ~basic_stringpiece_alloc() {
			clear_internal();
		}

		void clear() {
			clear_internal();
			sp_t::clear();
			m_capacity = 0;
		}

		template <typename Range>
		inline void assign(const Range &other, bool allocate_always = true) {
			clear_internal();
			init(other, allocate_always);
		}

		template <class A>
		void assign(const basic_stringpiece_alloc<CharT, A> &other) {
			if (!other.m_capacity) {
				clear_internal();
				sp_t::operator=(other);
				m_capacity = 0;
			} else {
				const size_type new_size = other.size();
				if (!m_capacity || this->size() < new_size) {
					clear_internal();
					init(other, true);
				} else {
					CharT *buf = const_cast<CharT*>(this->begin());
					char_traits::copy(buf, other.begin(), new_size + 1);
					buf += new_size + 1;
					while (buf <= this->end())
						this->destroy(buf);
				}
			}
		}

		template <class A>
		inline self_t &operator=(const basic_stringpiece_alloc<CharT, A> &other) {
			assign(other);
			return *this;
		}

		void swap(self_t &other) {
			::std::swap<sp_t>(*this, other);
			::std::swap(this->m_capacity, other.m_capacity);
		}

		const CharT *c_str() const { return this->begin(); }

	private:
		template <typename Range>
		void init(const Range &r, bool allocate_always) {
			if (allocate_always || detail::is_c_str(r)) {
				m_capacity = ::boost::size(r) + 1;
				typename ::boost::range_const_iterator<Range>::type begin = ::boost::begin(r);
				CharT *buf = this->allocate(m_capacity);
				this->first = buf; this->second = buf + (m_capacity - 1);
				while (buf != this->second)
					this->construct(buf++, *begin++);
				this->construct(buf, char_traits::to_char_type(0));
			} else {
				m_capacity = 0;
			}
		}

		void clear_internal() {
			if (m_capacity != 0) {
				CharT* buf = const_cast<CharT*>(this->begin());
				while (buf <= this->end())
					this->destroy(buf++);
				this->deallocate(const_cast<CharT*>(this->begin()), m_capacity);
			}
		}

		size_type m_capacity;
	};


	template <typename Iterator>
	inline
	basic_stringpiece<Iterator>
	make_stringpiece(const Iterator &begin, const Iterator &end)
	{
		return basic_stringpiece<Iterator>(begin, end);
	}


	template <typename Range>
	inline
	basic_stringpiece<typename ::boost::range_iterator<Range>::type>
	make_stringpiece(const Range &r)
	{
		return basic_stringpiece<typename ::boost::range_iterator<Range>::type>(r);
	}


#define UTIL_STRINGPIECE_SPECIALIZE_IMPL(name, base_type, ...) \
	extern template class base_type< __VA_ARGS__ >; \
	typedef base_type< __VA_ARGS__ > name;

#ifdef UTIL_STRINGPIECE_HPP_BUILD
#	define UTIL_STRINGPIECE_SPECIALIZE(name, base_type, ...) \
		UTIL_STRINGPIECE_SPECIALIZE_IMPL(name, base_type, __VA_ARGS__) \
		template class base_type< __VA_ARGS__ >;
#else
#	define UTIL_STRINGPIECE_SPECIALIZE(name, base_type, ...) \
		UTIL_STRINGPIECE_SPECIALIZE_IMPL(name, base_type, __VA_ARGS__)
#endif

	UTIL_STRINGPIECE_SPECIALIZE(stringpiece, basic_stringpiece, const char*)
	UTIL_STRINGPIECE_SPECIALIZE(wstringpiece, basic_stringpiece, const wchar_t*)
	UTIL_STRINGPIECE_SPECIALIZE(stringpiece_alloc, basic_stringpiece_alloc, char)
	UTIL_STRINGPIECE_SPECIALIZE(wstringpiece_alloc, basic_stringpiece_alloc, wchar_t)

#undef UTIL_STRINGPIECE_SPECIALIZE
#undef UTIL_STRINGPIECE_SPECIALIZE_IMPL

} /* namespace util */


template <typename CharT, class Allocator, typename Iterator>
inline
::std::basic_string<CharT, Allocator> &
operator+=(::std::basic_string<CharT, Allocator> &s1, const util::basic_stringpiece<Iterator> &s2)
{
	return s1.append(s2.begin(), s2.end());
}


template <typename CharT, class Allocator, typename Iterator>
inline bool operator==(::std::basic_string<CharT, Allocator> &s1, const util::basic_stringpiece<Iterator> &s2)
{
	return s2 == s1;
}


template <typename CharT, class Allocator, typename Iterator>
inline bool operator!=(::std::basic_string<CharT, Allocator> &s1, const util::basic_stringpiece<Iterator> &s2)
{
	return s2 != s1;
}


template <typename CharT, class Allocator, typename Iterator>
inline bool operator<(::std::basic_string<CharT, Allocator> &s1, const util::basic_stringpiece<Iterator> &s2)
{
	return s2 > s1;
}


template <typename CharT, class Allocator, typename Iterator>
inline bool operator<=(::std::basic_string<CharT, Allocator> &s1, const util::basic_stringpiece<Iterator> &s2)
{
	return s2 >= s1;
}


template <typename CharT, class Allocator, typename Iterator>
inline bool operator>(::std::basic_string<CharT, Allocator> &s1, const util::basic_stringpiece<Iterator> &s2)
{
	return s2 < s1;
}


template <typename CharT, class Allocator, typename Iterator>
inline bool operator>=(::std::basic_string<CharT, Allocator> &s1, const util::basic_stringpiece<Iterator> &s2)
{
	return s2 <= s1;
}


template <typename CharT, class Traits>
inline ::std::basic_ostream<CharT, Traits> &operator<<(
	::std::basic_ostream<CharT, Traits> &out,
	const util::basic_stringpiece<const CharT*> &s
){
	return out.write(s.begin(), s.size());
}

#endif /* UTIL_STRINGPIECE_HPP_ */
