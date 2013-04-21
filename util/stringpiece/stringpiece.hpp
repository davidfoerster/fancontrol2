/*
 * stringpiece.hpp
 *
 *  Created on: 24.09.2012
 *      Author: malte
 */

#pragma once
#ifndef UTIL_STRINGPIECE_HPP_
#define UTIL_STRINGPIECE_HPP_

#include "../algorithm.hpp"
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
#include <boost/type_traits/is_integral.hpp>
#include <boost/utility/enable_if.hpp>
#include <algorithm>
#include <iterator>
#include <utility>
#include <string>
#include <iosfwd>
#include <stdexcept>


#ifndef UTIL_STRINGPIECE_HPP_API
#	define UTIL_STRINGPIECE_HPP_API extern
#endif


namespace util {

	template <typename Iterator>
	class basic_stringpiece;


	namespace detail {

		template <typename Range>
		static
		typename ::boost::disable_if<
			::boost::is_same<
				typename ::boost::range_category<Range>::type,
				::std::random_access_iterator_tag
			>,
			bool
		>::type
		is_c_str(const Range &r);

		template <typename Range>
		static
		typename ::boost::enable_if<
			::boost::is_same<
			 typename ::boost::range_category<Range>::type,
				::std::random_access_iterator_tag
			>,
			bool
		>::type
		is_c_str(const Range &r);

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

		struct traits_type
			: ::std::char_traits<value_type>
		{
			typedef ::std::char_traits<value_type> underlying_type;

			static inline bool le(const value_type &a, const value_type &b) { return lt(a, b) || eq(a, b); }
			static inline bool gt(const value_type &a, const value_type &b) { return !le(a, b); }
			static inline bool ge(const value_type &a, const value_type &b) { return !lt(a, b); }
		};

		static const basic_stringpiece<Iterator> &empty_string();

	public:
		basic_stringpiece();

		basic_stringpiece(const Iterator &begin, const Iterator &end);

		basic_stringpiece(const ::std::pair<Iterator, Iterator> &other);

		template <typename U>
		explicit basic_stringpiece(const ::std::pair<U, U> &other);

		template <class Allocator>
		basic_stringpiece(const ::std::basic_string<value_type, typename traits_type::underlying_type, Allocator> &other);

		explicit basic_stringpiece(const value_type *s);

		basic_stringpiece(const value_type *s, size_type length);

		template < ::std::size_t Size>
		basic_stringpiece(const value_type (&s)[Size]);

		virtual ~basic_stringpiece();

		inline size_type size() const { return end() - begin(); }
		inline size_type length() const { return size(); }
		inline bool empty() const { return begin() == end(); }
		inline bool operator!() const { return empty(); }

		typename ::boost::enable_if<
			::boost::is_same<
				typename iterator_traits::iterator_category,
				::std::random_access_iterator_tag
			>,
			const value_type*
		>::type
		c_str() const;

		template <class Alloc>
		void str(::std::basic_string<value_type, typename traits_type::underlying_type, Alloc> &s) const;

		::std::basic_string<value_type> str() const;

		inline Iterator begin() const { return this->first; }
		inline Iterator end() const { return this->second; }

		const value_type &front() const;
		const value_type &back() const;

		inline const value_type &operator[](size_type i) const { return begin()[i]; }

		self_t substr(size_type pos) const;

		self_t substr(size_type pos, size_type n) const;

		void clear();

		template <typename Range>
		bool operator==(const Range &r) const;

		inline bool operator==(value_type *o) const;

		bool operator==(const value_type *o) const;

		template <typename Range>
		bool operator!=(const Range &r) const;

		template <typename Range>
		bool operator<(const Range &r) const;

		template <typename Range>
		bool operator<=(const Range &r) const;

		template <typename Range>
		bool operator>(const Range &r) const;

		template <typename Range>
		bool operator>=(const Range &r) const;

		template <typename Range>
		int compare(const Range &r) const;

		bool starts_with(const value_type *prefix) const;

		bool prefix_of(const value_type *s) const;

		template <typename Integer>
		typename boost::enable_if<boost::is_integral<Integer>, self_t>::type
		parse(Integer &, unsigned base = 10) const;
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
		typedef typename sp_t::traits_type traits_type;


		explicit basic_stringpiece_alloc(const Alloc &alloc = Alloc());

		basic_stringpiece_alloc(const self_t &other);

		template <class A>
		basic_stringpiece_alloc(const basic_stringpiece_alloc<CharT, A> &other, const Alloc &alloc = Alloc());

		template <typename Range>
		explicit basic_stringpiece_alloc(const Range &r, bool allocate_always = true, const Alloc &alloc = Alloc());

		virtual ~basic_stringpiece_alloc();

		void clear();

		template <typename Range>
		void assign(const Range &other, bool allocate_always = true);

		template <class A>
		void assign(const basic_stringpiece_alloc<CharT, A> &other);

		template <class A>
		self_t &operator=(const basic_stringpiece_alloc<CharT, A> &other);

		void swap(self_t &other);

		const CharT *c_str() const;

	private:
		template <typename Range>
		void init(const Range &r, bool allocate_always);

		void clear_internal();

		size_type m_capacity;
	};


	template <typename Iterator>
	basic_stringpiece<Iterator>
	make_stringpiece(const Iterator &begin, const Iterator &end);


	template <typename Range>
	basic_stringpiece<typename ::boost::range_iterator<Range>::type>
	make_stringpiece(const Range &r);

} // namespace util


namespace std {

	template <typename CharT, class Traits, class Allocator, typename Iterator>
	basic_string<CharT, Traits, Allocator> &
	operator+=(basic_string<CharT, Traits, Allocator> &s1, const util::basic_stringpiece<Iterator> &s2);


	template <typename CharT, class Traits>
	basic_ostream<CharT, Traits> &operator<<(
		basic_ostream<CharT, Traits> &out,
		const util::basic_stringpiece<const CharT*> &s
	);

} // namespace std



// implementations ============================================================

namespace util {

	template <typename Iterator>
	const basic_stringpiece<Iterator> &basic_stringpiece<Iterator>::empty_string()
	{
		static const basic_stringpiece<Iterator> &e = *new basic_stringpiece<Iterator>();
		return e;
	}


	template <typename Iterator>
	inline basic_stringpiece<Iterator>::basic_stringpiece()
	{ }


	template <typename Iterator>
	inline basic_stringpiece<Iterator>::basic_stringpiece(const Iterator &begin, const Iterator &end)
		: base_t(begin, end)
	{ }


	template <typename Iterator>
	inline basic_stringpiece<Iterator>::basic_stringpiece(const ::std::pair<Iterator, Iterator> &other)
		: base_t(other)
	{ }


	template <typename Iterator>
	template <typename U>
	inline basic_stringpiece<Iterator>::basic_stringpiece(const ::std::pair<U, U> &other)
		: base_t(other)
	{ }


	template <typename Iterator>
	template <class Allocator>
	inline basic_stringpiece<Iterator>::basic_stringpiece(
			const ::std::basic_string<value_type, typename traits_type::underlying_type, Allocator> &other)
		: base_t( (other.c_str(), base_t(other.data(), other.data()+other.size())) )
	{ }


	template <typename Iterator>
	inline basic_stringpiece<Iterator>::basic_stringpiece(const value_type *s)
		: base_t(s, s ? s + traits_type::length(s) : nullptr)
	{ }


	template <typename Iterator>
	inline basic_stringpiece<Iterator>::basic_stringpiece(const value_type *s, size_type length)
		: base_t(s, s + length)
	{ }


	template <typename Iterator>
	template < ::std::size_t Size>
	inline basic_stringpiece<Iterator>::basic_stringpiece(const value_type (&s)[Size])
		: base_t(s, s + Size - !s[Size-1])
	{ }


	template <typename Iterator>
	basic_stringpiece<Iterator>::~basic_stringpiece()
	{ }


	template <typename Iterator>
	inline
	typename ::boost::enable_if<
		::boost::is_same<
			typename basic_stringpiece<Iterator>::iterator_traits::iterator_category,
			::std::random_access_iterator_tag
		>,
		const typename basic_stringpiece<Iterator>::value_type*
	>::type
	basic_stringpiece<Iterator>::c_str() const
	{
		if (detail::is_c_str(*this))
			return &*begin();
		throw ::std::logic_error("cannot convert to C string");
	}


	template <typename Iterator>
	template <class Alloc>
	void basic_stringpiece<Iterator>::str(
			::std::basic_string<value_type, typename traits_type::underlying_type, Alloc> &s
	) const {
		if (!empty()) {
			s.assign(begin(), end());
		} else {
			s.clear();
		}
	}


	template <typename Iterator>
	inline
	::std::basic_string<typename basic_stringpiece<Iterator>::value_type>
	basic_stringpiece<Iterator>::str() const
	{
		return !empty() ?
			::std::basic_string<value_type>(begin(), end()) :
			::std::basic_string<value_type>();
	}


	template <typename Iterator>
	inline
	const typename basic_stringpiece<Iterator>::value_type &
	basic_stringpiece<Iterator>::front() const
	{
		return *begin();
	}


	template <typename Iterator>
	inline
	const typename basic_stringpiece<Iterator>::value_type &
	basic_stringpiece<Iterator>::back() const
	{
		return *(!empty() ? end()-1 : this->end());
	}


	template <typename Iterator>
	inline
	basic_stringpiece<Iterator>
	basic_stringpiece<Iterator>::substr(size_type pos) const
	{
		return self_t(begin() + pos, end());
	}


	template <typename Iterator>
	inline
	basic_stringpiece<Iterator>
	basic_stringpiece<Iterator>::substr(size_type pos, size_type n) const
	{
		const Iterator begin = this->begin() + pos;
		return self_t(begin, begin + n);
	}


	template <typename Iterator>
	inline void basic_stringpiece<Iterator>::clear()
	{
		this->second = this->first = iterator();
	}


	template <typename Iterator>
	template <typename Range>
	bool basic_stringpiece<Iterator>::operator==(const Range &r) const
	{
		return
			(&*::boost::begin(r) == &*this->begin()) ?
				&*::boost::end(r) == &*this->end() :
				::boost::equal(*this, r, traits_type::eq);
	}


	template <typename Iterator>
	inline bool basic_stringpiece<Iterator>::operator==(value_type *o) const
	{
		return operator==(const_cast<const value_type*>(o));
	}


	template <typename Iterator>
	bool basic_stringpiece<Iterator>::operator==(const value_type *o) const
	{
		iterator s = begin();

		if (o == &*s) {
			return !o || traits_type::length(o) == this->size();

		} else if (o) {
			for ( ; s != end() && *o; ++s, ++o) {
				if (!traits_type::eq(*s, *o))
					return false;
			}
			return s == end() && !*o;
		} else {
			return false;
		}
	}


	template <typename Iterator>
	template <typename Range>
	inline bool basic_stringpiece<Iterator>::operator!=(const Range &r) const
	{
		return !operator==(r);
	}


	template <typename Iterator>
	template <typename Range>
	bool basic_stringpiece<Iterator>::operator<(const Range &r) const
	{
		return ::boost::lexicographical_compare(*this, r, &traits_type::lt);
	}


	template <typename Iterator>
	template <typename Range>
	bool basic_stringpiece<Iterator>::operator<=(const Range &r) const
	{
		return ::boost::lexicographical_compare(*this, r, &traits_type::le);
	}


	template <typename Iterator>
	template <typename Range>
	bool basic_stringpiece<Iterator>::operator>(const Range &r) const
	{
		return ::boost::lexicographical_compare(*this, r, &traits_type::gt);
	}


	template <typename Iterator>
	template <typename Range>
	bool basic_stringpiece<Iterator>::operator>=(const Range &r) const
	{
		return ::boost::lexicographical_compare(*this, r, &traits_type::ge);
	}


	template <typename Iterator>
	template <typename Range>
	int basic_stringpiece<Iterator>::compare(const Range &r) const
	{
		const ::std::pair<Iterator, typename ::boost::range_const_iterator<Range>::type>
			mismatch = ::boost::mismatch(*this, r);
		const value_type
			c1 = (mismatch.first != this->end()) ? *mismatch.first : static_cast<value_type>(0),
			c2 = (mismatch.second != ::boost::end(r)) ? *mismatch.second : static_cast<value_type>(0);
		return util::compare(c1, c2, &traits_type::lt);
	}


	template <typename Iterator>
	bool basic_stringpiece<Iterator>::starts_with(const value_type *prefix) const
	{
		Iterator first = begin(), last = end();
		for ( ; first != last && *prefix != 0; ++first, ++prefix) {
			if (*first != *prefix)
				return false;
		}
		return first != last || *prefix == 0;
	}


	template <typename Iterator>
	bool basic_stringpiece<Iterator>::prefix_of(const value_type *s) const
	{
		Iterator first = begin(), last = end();
		for ( ; first != last && *s != 0; ++first, ++s) {
			if (*first != *s)
				return false;
		}
		return first == last;
	}


	/*
	template <typename Iterator>
	template <typename Integer>
	typename boost::enable_if< boost::is_integral<Integer>, basic_stringpiece<Iterator> >::type
	basic_stringpiece<Iterator>::operator>>(Integer &n_, unsigned base) const
	{
		const_iterator it = begin();
		Integer n = 0;
		while (it != end() && util::in_range<value_type>(*it, '0', '9' + 1)) {
			n = n * base +
		}
	}
	*/


	template <typename C, class A>
	inline basic_stringpiece_alloc<C,A>::basic_stringpiece_alloc(const A &alloc)
		: sp_t(nullptr, nullptr)
		, A(alloc)
		, m_capacity(0)
	{ }


	template <typename C, class A>
	inline basic_stringpiece_alloc<C,A>::basic_stringpiece_alloc(const self_t &other)
		: A(other)
	{
		init(other, !!other.m_capacity);
	}


	template <typename C, class A>
	template <class A2>
	inline basic_stringpiece_alloc<C,A>::basic_stringpiece_alloc(
			const basic_stringpiece_alloc<C, A2> &other, const A &alloc)
		: A(alloc)
	{
		init(other, !!other.m_capcacity);
	}


	template <typename C, class A>
	template <typename Range>
	inline basic_stringpiece_alloc<C,A>::basic_stringpiece_alloc(
			const Range &r, bool allocate_always, const A &alloc)
		: A(alloc)
	{
		init(r, allocate_always);
	}


	template <typename C, class A>
	basic_stringpiece_alloc<C,A>::~basic_stringpiece_alloc()
	{
		clear_internal();
	}


	template <typename C, class A>
	void basic_stringpiece_alloc<C,A>::clear()
	{
		clear_internal();
		sp_t::clear();
		m_capacity = 0;
	}


	template <typename C, class A>
	template <typename Range>
	inline void basic_stringpiece_alloc<C,A>::assign(const Range &other, bool allocate_always)
	{
		clear_internal();
		init(other, allocate_always);
	}


	template <typename C, class A>
	template <class A2>
	void basic_stringpiece_alloc<C,A>::assign(const basic_stringpiece_alloc<C, A2> &other)
	{
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
				value_type *buf = const_cast<value_type*>(this->begin());
				traits_type::copy(buf, other.begin(), new_size + 1);
				buf += new_size + 1;
				while (buf <= this->end())
					this->destroy(buf);
			}
		}
	}


	template <typename C, class A>
	template <class A2>
	inline
	basic_stringpiece_alloc<C,A> &
	basic_stringpiece_alloc<C,A>::operator=(const basic_stringpiece_alloc<C, A2> &other)
	{
		assign(other);
		return *this;
	}


	template <typename C, class A>
	inline void basic_stringpiece_alloc<C,A>::swap(self_t &other)
	{
		::std::swap<sp_t>(*this, other);
		::std::swap(this->m_capacity, other.m_capacity);
	}


	template <typename C, class A>
	inline const C *basic_stringpiece_alloc<C,A>::c_str() const
	{
		return this->begin();
	}


	template <typename C, class A>
	template <typename Range>
	void basic_stringpiece_alloc<C,A>::init(const Range &r, bool allocate_always)
	{
		if (allocate_always || detail::is_c_str(r)) {
			m_capacity = ::boost::size(r) + 1;
			typename ::boost::range_const_iterator<Range>::type begin = ::boost::begin(r);
			value_type *buf = this->allocate(m_capacity);
			this->first = buf; this->second = buf + (m_capacity - 1);
			while (buf != this->second)
				this->construct(buf++, *begin++);
			this->construct(buf, traits_type::to_char_type(0));
		} else {
			m_capacity = 0;
		}
	}


	template <typename C, class A>
	void basic_stringpiece_alloc<C,A>::clear_internal()
	{
		if (m_capacity != 0) {
			value_type* buf = const_cast<value_type*>(this->begin());
			while (buf <= this->end())
				this->destroy(buf++);
			this->deallocate(const_cast<value_type*>(this->begin()), m_capacity);
		}
	}


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

	} // namespace detail


	#define UTIL_STRINGPIECE_SPECIALIZE(name, base_type, ...) \
		UTIL_STRINGPIECE_HPP_API template class base_type< __VA_ARGS__ >; \
		typedef base_type< __VA_ARGS__ > name;

	UTIL_STRINGPIECE_SPECIALIZE(stringpiece, basic_stringpiece, const char*)
	UTIL_STRINGPIECE_SPECIALIZE(wstringpiece, basic_stringpiece, const wchar_t*)
	UTIL_STRINGPIECE_SPECIALIZE(stringpiece_alloc, basic_stringpiece_alloc, char)
	UTIL_STRINGPIECE_SPECIALIZE(wstringpiece_alloc, basic_stringpiece_alloc, wchar_t)

	#undef UTIL_STRINGPIECE_SPECIALIZE

} // namespace util


namespace std {

	template <typename CharT, class Traits, class Allocator, typename Iterator>
	inline
	basic_string<CharT, Traits, Allocator> &
	operator+=(basic_string<CharT, Traits, Allocator> &s1, const util::basic_stringpiece<Iterator> &s2)
	{
		return s1.append(s2.begin(), s2.end());
	}


	template <typename CharT, class Traits>
	inline basic_ostream<CharT, Traits> &operator<<(
		basic_ostream<CharT, Traits> &out,
		const util::basic_stringpiece<const CharT*> &s
	){
		return out.write(s.begin(), s.size());
	}

} // namespace std

#undef UTIL_STRINGPIECE_HPP_API
#endif /* UTIL_STRINGPIECE_HPP_ */
