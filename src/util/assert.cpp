/*
 * assert.cpp
 *
 *  Created on: 25.01.2012
 *      Author: malte
 */

#ifndef _GNU_SOURCE
	#define _GNU_SOURCE
#endif
#include "assert.hpp"
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <sys/ioctl.h>


#ifndef ASSERT_MAX_STACK_SIZE
	#define ASSERT_MAX_STACK_SIZE (1 << 8)
#endif


namespace util {
namespace assertion {
namespace detail {

void assert_printf_fail(
		const char *expr,
		const char *function, const char *file, unsigned int line,
		std::FILE *dst, const char *format, ...)
{
	::flockfile(dst);

	std::fprintf(dst, "%s:%u: assertion `%s' failed in function %s",
		file, line, expr, function);
	if (format)
	{
		::fputc_unlocked(':', dst);
		::fputc_unlocked(' ', dst);

		std::va_list va; va_start(va, format);
		vfprintf(dst, format, va);
		va_end(va);
	}
	else
	{
		::fputc_unlocked('.', dst);
	}

	::fputs_unlocked("\n\nStack trace:\n", dst);
	print_backtrace(dst, 1);

	::funlockfile(dst);
	std::abort();
}


void assert_perror_fail(
		const char *expr,
		const char *function, const char *file, unsigned int line,
		std::FILE *dst, int errnum)
{
	assert_printf_fail(expr, function, file, line, dst, "%s (errno=%i)", std::strerror(errnum), errnum);
}

}  // namespace detail


void print_backtrace( std::FILE *dst, int skip )
{
	skip = (skip >= 0) ? skip + 1 : 0;
	void *stack_frames[ASSERT_MAX_STACK_SIZE];
	const int stack_frames_count =
		::backtrace(stack_frames, ASSERT_MAX_STACK_SIZE);
	if (stack_frames_count <= skip)
		return;

	::flockfile(dst);
	char *demangled = nullptr;
	std::size_t demangled_size = 0;
	::winsize term;
	if (::ioctl(::fileno_unlocked(dst), TIOCGWINSZ, &term) != 0)
		term = { 0 };

	for (int i = skip; i < stack_frames_count; i++)
	{
		::Dl_info dl_info;
		if (::dladdr(stack_frames[i], &dl_info))
		{
			int status;
			demangled = ::abi::__cxa_demangle(
				dl_info.dli_sname, demangled, &demangled_size, &status);
			int charcount = std::fprintf(
				dst, "%3d %*p  %s + %tx\n",
				i, static_cast<int>(sizeof(void*) * 2 + 2), stack_frames[i],
				status ? dl_info.dli_sname : demangled,
				static_cast<const char*>(stack_frames[i])
					- static_cast<const char*>(dl_info.dli_saddr));

			if (term.ws_col > 0 && charcount - 1 >= term.ws_col)
				::fputc_unlocked('\n', dst);
		}
		else
		{
			std::fprintf(dst, "%3d %*p\n",
				i, static_cast<int>(sizeof(void*) * 2 + 2), stack_frames[i]);
		}
	}

	if (stack_frames_count == ASSERT_MAX_STACK_SIZE)
		::fputs_unlocked("... maybe more ...\n", dst);

	std::free(demangled);
	::funlockfile(dst);
}

} // namespace assertion
} // namespace util
