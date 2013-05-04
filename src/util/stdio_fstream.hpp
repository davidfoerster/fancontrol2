/*
 * stdio_fstream.hpp
 *
 *  Created on: 03.05.2013
 *      Author: malte
 */

#pragma once
#ifndef UTIL_STDIO_FSTREAM_HPP_
#define UTIL_STDIO_FSTREAM_HPP_

#include "util/algorithm.hpp"
#include <ext/stdio_filebuf.h>
#include <fcntl.h>


#ifndef UTIL_STDIO_FSTREAM_API
#	define UTIL_STDIO_FSTREAM_API extern
#endif


namespace util {

namespace detail {

	constexpr
	int get_stdio_openmode( std::ios::openmode mode, int additional_flags = 0 );

}


template <typename CharT, class Traits = std::char_traits<CharT> >
class basic_stdio_fstream
	: public std::basic_iostream<CharT, Traits>
{
	typedef std::basic_iostream<CharT, Traits> super;

public:
	class stdio_filebuf
		: public __gnu_cxx::stdio_filebuf<CharT, Traits>
	{
	public:
		stdio_filebuf() = default;

		virtual ~stdio_filebuf() = default;

		stdio_filebuf( int __fd, std::ios_base::openmode __mode,
				std::size_t __size );

		stdio_filebuf *open( int __fd, std::ios_base::openmode __mode,
				std::size_t __size );

		std::ios::openmode mode() const;

	};


	basic_stdio_fstream();

	basic_stdio_fstream( const char *pathname, std::ios::openmode ios_mode,
			int std_flags_additional = 0, ::mode_t std_mode = 0,
			std::size_t bufsize = BUFSIZ );

	virtual ~basic_stdio_fstream();

	inline bool is_open() const;

	void open( const char *pathname, std::ios::openmode ios_mode,
			int std_flags_additional = 0, ::mode_t std_mode = 0,
			std::size_t bufsize = BUFSIZ );

	void close();

	std::ios::openmode mode() const;

protected:
	void init();

	stdio_filebuf m_pbuf;

};


typedef basic_stdio_fstream<char> stdio_fstream;

} /* namespace util */



// implementations ========================================
namespace util {

namespace detail {

	constexpr
	int get_stdio_openmode( std::ios::openmode mode, int additional_flags )
	{
		using std::ios;
		return (additional_flags & ~static_cast<int>(O_ACCMODE))
			| ((mode & ios::out) ? ((mode & ios::in) ? O_RDWR : O_WRONLY) : O_RDONLY)
			| util::convert_flagbit< ios::openmode, ios::app, int, O_APPEND >(mode)
			| util::convert_flagbit< ios::openmode, ios::trunc, int, O_TRUNC >(mode)
			;
	}

}


template <typename CharT, class Traits>
basic_stdio_fstream<CharT,Traits>::stdio_filebuf::stdio_filebuf(
		int __fd, std::ios_base::openmode __mode, std::size_t __size )
{
	this->open( __fd, __mode, __size );
}


template <typename CharT, class Traits>
typename basic_stdio_fstream<CharT,Traits>::stdio_filebuf *
basic_stdio_fstream<CharT,Traits>::stdio_filebuf::open(
		int __fd, std::ios_base::openmode __mode, std::size_t __size )
{
	if (!this->is_open())
	{
		this->_M_file.sys_open(__fd, __mode);
		if (this->is_open())
		{
		  this->_M_mode = __mode;
		  this->_M_buf_size = __size;
		  this->_M_allocate_internal_buffer();
		  this->_M_reading = false;
		  this->_M_writing = false;
		  this->_M_set_buffer(-1);
		  return this;
		}
	}
	return nullptr;
}


template <typename CharT, class Traits>
std::ios::openmode
basic_stdio_fstream<CharT,Traits>::stdio_filebuf::mode() const
{
	return this->is_open() ? this->_M_mode : static_cast<std::ios::openmode>(0);
}


template <typename CharT, class Traits>
basic_stdio_fstream<CharT,Traits>::basic_stdio_fstream()
	: super(nullptr)
{
	this->init();
}


template <typename CharT, class Traits>
basic_stdio_fstream<CharT,Traits>::basic_stdio_fstream(
		const char *pathname, std::ios::openmode ios_mode,
		int std_flags_additional, ::mode_t std_mode, std::size_t bufsize )
	: super(nullptr)
{
	this->open(pathname, ios_mode, std_flags_additional, std_mode, bufsize);
	this->init();
}


template <typename CharT, class Traits>
basic_stdio_fstream<CharT,Traits>::~basic_stdio_fstream()
{
	this->exceptions(std::ios::goodbit);
	this->rdbuf(nullptr);
}


template <typename CharT, class Traits>
inline bool basic_stdio_fstream<CharT,Traits>::is_open() const
{
	return m_pbuf.is_open();
}


template <typename CharT, class Traits>
void basic_stdio_fstream<CharT,Traits>::open(
		const char *pathname, std::ios::openmode ios_mode,
		int std_flags_additional, ::mode_t std_mode, std::size_t bufsize )
{
	if (!m_pbuf.is_open()) {
		int fd = ::open(pathname,
				detail::get_stdio_openmode(ios_mode, std_flags_additional),
				std_mode);
		if (fd >= 0) {
			if (m_pbuf.open(fd, ios_mode, bufsize)) {
				this->clear();
				return;
			} else {
				::close(fd);
			}
		}
	}
	this->setstate(std::ios::failbit);
}


template <typename CharT, class Traits>
void basic_stdio_fstream<CharT,Traits>::close()
{
	if (!m_pbuf.close())
		this->setstate(std::ios::failbit);
}


template <typename CharT, class Traits>
std::ios::openmode basic_stdio_fstream<CharT,Traits>::mode() const
{
	return (this->super::rdbuf() == &m_pbuf) ?
			m_pbuf.mode() :
			static_cast<std::ios::openmode>(0);
}


template <typename CharT, class Traits>
inline void basic_stdio_fstream<CharT,Traits>::init()
{
	this->rdbuf(&m_pbuf);
}


UTIL_STDIO_FSTREAM_API template class basic_stdio_fstream<char>;

#undef UTIL_STRINGPIECE_HPP_API

} /* namespace util */
#endif /* UTIL_STDIO_FSTREAM_HPP_ */
