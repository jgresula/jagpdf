/*
 * Copyright (c) 2005-2009 Jaroslav Gresula
 *
 * Distributed under the MIT license (See accompanying file
 * LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
 *

 */
#include <jagpdf/api.h>
#include "ctestcommon.h"

int version(int argc, char ** const argv)
{
    jag_UInt major, minor, patch;
    jag_UInt version = jag_version();

    JAG_IGNORE_UNUSED(argc);
    JAG_IGNORE_UNUSED(argv);

    major = version >> 16;
    minor = (version >> 8) & 0xff;
    patch = version & 0xff;

    JAGC_TEST(major == jag_this_version_major);
    JAGC_TEST(minor == jag_this_version_minor);
    JAGC_TEST(patch == jag_this_version_patch);

    return 0;
}

/** EOF @file */
