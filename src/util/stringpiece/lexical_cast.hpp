/*
 * lexical_cast.hpp
 *
 *  Created on: 15.10.2012
 *      Author: malte
 */

#pragma once
#ifndef UTIL_STRINGPIECE_LEXICAL_CAST_HPP_
#define UTIL_STRINGPIECE_LEXICAL_CAST_HPP_

#include "util/stringpiece/stringpiece.hpp"
#include "util/in_range.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/range/size.hpp>
#include <boost/assert.hpp>
#include <istream>
#include <utility>


#ifndef UTIL_STRINGPIECE_LEXICAL_CAST_API
#	define UTIL_STRINGPIECE_LEXICAL_CAST_API extern
#endif


namespace util {

typedef std::pair< std::ios::iostate, std::streampos > streamstate;


namespace detail {

template <typename Iterator>
class stringpiece_streambuf
	: public std::basic_streambuf<typename std::iterator_traits<Iterator>::value_type>
{
private:
	typedef std::basic_streambuf<typename std::iterator_traits<Iterator>::value_type> super;

	basic_stringpiece<Iterator> str;
	typename super::off_type pos;

public:
	typedef typename super::char_type char_type;
	typedef typename super::traits_type traits_type;
	typedef typename super::int_type int_type;
	typedef typename super::pos_type pos_type;
	typedef typename super::off_type off_type;

	explicit stringpiece_streambuf(const basic_stringpiece<Iterator> &);

	virtual ~stringpiece_streambuf();

protected:
	virtual int_type underflow();

	virtual int_type uflow();

	virtual pos_type seekoff(off_type offset, std::ios::seekdir which, std::ios::openmode mode);
};


template <typename CharT>
struct stringpiece_streambuf<const CharT*>
	: std::basic_streambuf<CharT>
{
private:
	typedef std::basic_streambuf<CharT> super;

public:
	typedef typename super::char_type char_type;
	typedef typename super::traits_type traits_type;
	typedef typename super::int_type int_type;
	typedef typename super::pos_type pos_type;
	typedef typename super::off_type off_type;

	explicit stringpiece_streambuf(const basic_stringpiece<const CharT*> &);

	virtual ~stringpiece_streambuf();

protected:
	virtual pos_type seekoff(off_type offset, std::ios::seekdir which, std::ios::openmode mode);

	virtual pos_type seekpos(pos_type pos, std::ios::openmode mode);
};

} // namespace detail
} // namespace util


namespace boost {

template <typename Target, typename Iterator>
Target lexical_cast(const util::basic_stringpiece<Iterator> &src,
		std::basic_istream<typename util::basic_stringpiece<Iterator>::value_type> &istream,
		util::streamstate *streamstate = nullptr);


template <typename Target, typename Iterator>
Target lexical_cast(const util::basic_stringpiece<Iterator> &src, util::streamstate *streamstate = nullptr);

} // namespace boost


namespace util {
	using boost::lexical_cast;
}



// implementations ========================================

namespace util {
	namespace detail {

template <typename It>
inline
stringpiece_streambuf<It>::stringpiece_streambuf(const basic_stringpiece<It> &str)
	: str(str)
	, pos(0)
{ }


template <typename It>
stringpiece_streambuf<It>::~stringpiece_streambuf()
{ }


template <typename It>
typename stringpiece_streambuf<It>::int_type
stringpiece_streambuf<It>::underflow()
{
	return (str.first != str.second) ?
		traits_type::to_int_type(*str.first) :
		traits_type::eof();
}


template <typename It>
typename stringpiece_streambuf<It>::int_type
stringpiece_streambuf<It>::uflow()
{
	if (str.first != str.second) {
		pos++;
		return traits_type::to_int_type(*str.first++);
	} else {
		return traits_type::eof();
	}
}


template <typename It>
typename stringpiece_streambuf<It>::pos_type
stringpiece_streambuf<It>::seekoff(off_type offset, std::ios::seekdir which, std::ios::openmode mode)
{
	using std::ios;
	if (offset == 0 && which == ios::cur && mode == ios::in) {
		return static_cast<pos_type>(pos);
	}
	return static_cast<pos_type>(static_cast<off_type>(-1));
}


template <typename CharT>
inline
stringpiece_streambuf<const CharT*>::stringpiece_streambuf(const basic_stringpiece<const CharT*> &str)
{
	this->setg(
		const_cast<CharT*>(str.begin()),
		const_cast<CharT*>(str.begin()),
		const_cast<CharT*>(str.end()));
}


template <typename CharT>
stringpiece_streambuf<const CharT*>::~stringpiece_streambuf()
{ }


template <typename CharT>
typename stringpiece_streambuf<const CharT*>::pos_type
stringpiece_streambuf<const CharT*>::seekoff(off_type offset, std::ios::seekdir which, std::ios::openmode mode)
{
	using std::ios;
	using util::in_range;

	if (offset == 0) {
		return pos_type(this->gptr() - this->eback());
	}
	if (mode == ios::in) {
		char_type *ept[3];
		ept[ios::beg] = this->eback();
		ept[ios::cur] = this->gptr();
		ept[ios::end] = this->egptr();

		ept[ios::cur] = ept[which] + offset;
		if (in_range(ept[ios::cur], ept[ios::beg], ept[ios::end] + 1)) {
			const off_type pos = ept[ios::cur] - ept[ios::beg];
			this->setg(ept[ios::beg], ept[ios::cur], ept[ios::end]);
			return pos;
		}
	}

	return static_cast<pos_type>(static_cast<off_type>(-1));
}


template <typename CharT>
typename stringpiece_streambuf<const CharT*>::pos_type
stringpiece_streambuf<const CharT*>::seekpos(pos_type pos, std::ios::openmode mode)
{
	using std::ios;
	using util::in_range;

	if (pos == 0)
		return 0;

	if (mode == ios::in) {
		char_type *first = this->eback(),
			*new_next = first + pos,
			*last = this->egptr();
		if (in_range(new_next, first, last + 1)) {
			if (new_next != this->gptr())
				this->setg(first, new_next, last);
			return pos;
		}
	}

	return static_cast<pos_type>(static_cast<off_type>(-1));
}


UTIL_STRINGPIECE_LEXICAL_CAST_API template class stringpiece_streambuf<const char*>;
UTIL_STRINGPIECE_LEXICAL_CAST_API template class stringpiece_streambuf<const wchar_t*>;

	}  // namespace detail
} // namespace util


namespace boost {

template <typename Target, typename Iterator>
Target
lexical_cast(const util::basic_stringpiece<Iterator> &src,
		std::basic_istream<typename util::basic_stringpiece<Iterator>::value_type> &istream,
		util::streamstate *streamstate)
{
	using std::ios;
	typedef typename util::basic_stringpiece<Iterator>::value_type char_type;

	util::detail::stringpiece_streambuf<Iterator> streambuf(src);
	std::basic_streambuf<char_type> *const previous_streambuf = istream.rdbuf(&streambuf);
	istream.exceptions(
#ifdef NDEBUG
			ios::badbit
#else
			ios::goodbit
#endif
		);

	Target result;
	istream >> result;

	const ios::iostate rdstate = istream.rdstate();
	BOOST_ASSERT(!(rdstate & ios::badbit));

	if (streamstate) {
		istream.unsetf(ios::skipws);
		streamstate->second = istream.tellg();
		streamstate->first = rdstate;
	}

	if (previous_streambuf)
		istream.rdbuf(previous_streambuf);

	if (!(rdstate & (ios::failbit | ios::badbit)))
		return result;

	BOOST_THROW_EXCEPTION(bad_lexical_cast(typeid(src), typeid(Target)));
}


template <typename Target, typename Iterator>
inline
Target
lexical_cast(const util::basic_stringpiece<Iterator> &src, util::streamstate *streamstate)
{
	std::basic_istream<typename util::basic_stringpiece<Iterator>::value_type> istream(nullptr);
	return lexical_cast<Target, Iterator>(src, istream, streamstate);
}

} // namespace boost

#undef UTIL_STRINGPIECE_LEXICAL_CAST_API
#endif /* UTIL_STRINGPIECE_LEXICAL_CAST_HPP_ */
