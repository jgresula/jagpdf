/*
 * Copyright (c) 2005-2009 Jaroslav Gresula
 *
 * Distributed under the MIT license (See accompanying file
 * LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
 *

 */
#include <stdio.h>
#include <jagpdf/api.h>
#include <stdlib.h>

#include "ctestcommon.h"
#include <assert.h>

int basic(int argc, char ** const argv)
{
    char buffer[512];
    jag_Document doc;
    jag_Profile cfg;

    JAG_IGNORE_UNUSED(argc);

    cfg = jag_create_profile();
    jag_Profile_set(cfg, "general.deflated_streams", "0");

    sprintf(buffer, "%s/basic.pdf" ,argv[1]);
    doc = jag_create_file(buffer, cfg);

    if (!doc)
    {
        puts(jag_error_msg());
        return 1;
    }

    /*
    !! causes core
    doc = jag_create_file(argv[1], 0);
    */

    jag_Document_page_start(doc, 5.9*72, 3.5*72);
    jag_Document_page_end(doc);
    jag_Document_finalize(doc);

    jag_release(doc);
    jag_release(cfg);

    return 0;
}



/** EOF @file */
