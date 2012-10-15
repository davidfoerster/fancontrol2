/*
 * assert.cpp
 *
 *  Created on: 25.01.2012
 *      Author: malte
 */

#include "assert.hpp"
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <cstdio>

namespace util {
	namespace assertion {
	    namespace detail {

		    void assert_printf_fail(
				    const char *expr,
				    const char *function, const char *file, unsigned int line,
				    ::std::FILE *_dst, const char *format, ...)
			    throw()
		    {
			    using namespace ::std;
			    FILE *dst = static_cast<FILE*>(_dst);
			    flockfile(dst);

			    fprintf(dst, "%s:%u: assertion `%s' failed in function %s", file, line, expr, function);
			    if (format) {
				    fputc_unlocked(':', dst);
				    fputc_unlocked(' ', dst);

				    va_list va; va_start(va, format);
				    vfprintf(dst, format, va);
				    va_end(va);
			    } else {
				    fputc_unlocked('.', dst);
			    }
			    fputc_unlocked('\n', dst);

			    funlockfile(dst);
			    abort();
		    }


		    void assert_perror_fail(
				    const char *expr,
				    const char *function, const char *file, unsigned int line,
				    ::std::FILE *dst, int errnum)
			    throw()
		    {
			    assert_printf_fail(expr, function, file, line, dst, "%s (errno=%i)", ::std::strerror(errnum), errnum);
		    }

	    }
	}
}
