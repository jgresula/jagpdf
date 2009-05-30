// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//

#ifndef TESTTOOLS_JG1524_H__
#define TESTTOOLS_JG1524_H__
#include <core/errlib/except.h>
#include <core/generic/unused.h>
#include <boost/detail/lightweight_test.hpp>
#include <stdexcept>

#if defined(_WIN32)
# include <windows.h>
#endif

#define JAG_MUST_THROW_EX(exp, exception_, msg)                               \
    try {                                                                       \
        exp;                                                                    \
        BOOST_ERROR("Exception expected");                                    \
    } catch(exception_& exc) {                                                \
        BOOST_TEST(exc.msg_id() == msg::msg_id());                            \
    }


#ifdef SHOW_CAUGHT_EXCEPTION
# define SHOW_CAUGHT_EXCEPTION__(exc)                        \
    std::cerr << "Caught Expected Exception:\n";             \
    jag::output_exception(exc, std::cerr);                 \
    std::cerr << std::endl;
#else
# define SHOW_CAUGHT_EXCEPTION__(exc)
#endif


#define JAG_MUST_THROW(exp, exception_ )                   \
    try {                                                    \
        exp;                                                 \
        BOOST_ERROR("Exception expected");                 \
    } catch(exception_& exc) {                             \
        jag::detail::ignore_unused(exc);                     \
        SHOW_CAUGHT_EXCEPTION__(exc)                         \
    }


//
//
//
inline int guarded_test_run(void (*test_fn)())
{
    try
    {
#if defined(_WIN32)
        // no modal dialog in case of crash
        DWORD dwMode = ::SetErrorMode(SEM_NOGPFAULTERRORBOX);
        ::SetErrorMode(dwMode | SEM_NOGPFAULTERRORBOX);
        // assertion messages to stderr
        _set_error_mode(_OUT_TO_STDERR);
        // no win dialog on abort
        _set_abort_behavior(0, _WRITE_ABORT_MSG);
#endif
        test_fn();
    }
    catch(jag::exception const& exc)
    {
        jag::output_exception(exc, std::cerr);
        return 1;
    }
    catch(std::exception const& exc)
    {
        std::cerr << exc.what();
        return 1;
    }

    return 0;
}


#endif // TESTTOOLS_JG1524_H__
/** EOF @file */
