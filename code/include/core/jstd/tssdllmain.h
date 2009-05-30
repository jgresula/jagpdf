// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef TSSDLLMAIN_JG1316_H__
#define TSSDLLMAIN_JG1316_H__

// This file implements correct TLS initialization/release. It should be
// included exactly once in a shared library.
//
// If included more than once then the linker should issue an error regarding
// multiply defined symbols.

#include <core/jstd/tss.h>
#include <core/generic/unused.h>

#ifndef _WIN32
// ---------------------------------------------------------------------------
//                 GCC speficic initialization (in Unix)
//
#  if defined(__GNUC__)
static void gcc_dll_attach() __attribute__ ((constructor));
static void gcc_dll_detach() __attribute__ ((destructor));

static
void gcc_dll_attach()
{
    if (!jag::jstd::tls_on_process_attach())
        abort();
}

static
void gcc_dll_detach()
{
    jag::jstd::tls_on_process_detach();
}

namespace jag { namespace jstd {
    // this confirms that tls hooks are called
    void tls_cleanup_implemented() {}
}}


#  endif // __GNUC__
// ---------------------------------------------------------------------------
//                    WIN32 speficic initialization
//
#else // not _WIN32

# include <stdlib.h>
# include <core/generic/assert.h>
# include <windows.h>

int __stdcall DllMain(void* hinst, unsigned reason,  void* reserved)
{
    JAG_ASSERT(reason < 4);
    jag::detail::ignore_unused(hinst);
    jag::detail::ignore_unused(reserved);

    bool result = true;

    switch(reason)
    {
    case 1: { // process attach
        char const* no_dialogs = getenv("JAG_NO_MODAL_DIALOGS");
        if (no_dialogs)
        {
            // send assertion messages to stderr uncoditionally
            _set_error_mode(_OUT_TO_STDERR);
            // disable the crash dialog
            // http://blogs.msdn.com/oldnewthing/archive/2004/07/27/198410.aspx
            DWORD dwMode = ::SetErrorMode(SEM_NOGPFAULTERRORBOX);
            ::SetErrorMode(dwMode | SEM_NOGPFAULTERRORBOX);
            // no Win dialog on abort()
            _set_abort_behavior(0, _WRITE_ABORT_MSG);
        }
        result = jag::jstd::tls_on_process_attach();
        break;
    }

    case 0: // process detach
        result = jag::jstd::tls_on_process_detach();
        break;

    case 2: // thread attach
        result = jag::jstd::tls_on_thread_attach();
        break;

    case 3: // thread detach
        result = jag::jstd::tls_on_thread_detach();
        break;
    }

    return result ? 1 : 0;
}

namespace jag { namespace jstd {
    // this confirms that tls hooks are called
    void tls_cleanup_implemented() {}
}}

#endif // _WIN32

#endif // TSSDLLMAIN_JG1316_H__
/** EOF @file */
