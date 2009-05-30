/*
 * Copyright (c) 2005-2009 Jaroslav Gresula
 *
 * Distributed under the MIT license (See accompanying file
 * LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
 *

 */
#include "ctestcommon.h"


int errhandling(int argc, char ** const argv)
{
    jag_Document doc;
    jag_Profile cfg;

    jag_streamout stream_out = get_noop_stream();

    JAG_IGNORE_UNUSED(argc);
    JAG_IGNORE_UNUSED(argv);

    cfg = jag_create_profile();
    if (!jag_create_file("/this/file/does/not/exist", cfg)) {
        /* OK - really failed */
    } else {
        JAGC_FAIL;
    }

    doc = jag_create_stream(&stream_out, cfg);
    JAGC_TEST(doc);

    if (!jag_Document_image_load(doc,0)) {
        /* OK - really failed */
    } else {
        JAGC_FAIL;
    }

    if (jag_Document_page_end(doc)) {
        /* OK - really failed */
    } else {
        JAGC_FAIL;
    }

    jag_release(doc);
    jag_release(cfg);

    return 0;
}



/** EOF @file */
