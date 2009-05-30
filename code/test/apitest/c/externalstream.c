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

static
jag_Int JAG_CALLSPEC write(void* custom, void const* data, jag_ULong size)
{
    FILE *f = (FILE*)custom;
     if (size != fwrite(data, 1, size, f))
        return 1;

    return 0;
}

static
jag_Int JAG_CALLSPEC close(void* custom)
{
    FILE *f = (FILE*)custom;
    if (fclose(f))
        return 1;

    return 0;
}

static
void report_error()
{
    puts(jag_error_msg());
    exit(1);
}

int externalstream(int argc, char ** const argv)
{
    char buffer[512];
    jag_Document doc;
    jag_Profile cfg;
    jag_streamout stream_out;
    JAG_IGNORE_UNUSED(argc);

    stream_out.write = write;
    stream_out.close = close;
    sprintf(buffer, "%s/basic_extstream.pdf" ,argv[1]);
    stream_out.custom_data = (void*)fopen(buffer, "wb");

    cfg = jag_create_profile();
    jag_Profile_set(cfg, "general.deflated_streams", "0");
    doc = jag_create_stream(&stream_out, cfg);
    jag_release(cfg);

    if (!doc)
        report_error();


    jag_Document_page_start(doc, 5.9*72, 3.5*72);
    jag_Document_page_end(doc);
    jag_Document_finalize(doc);
    jag_release(doc);

    return 0;
}

/** EOF @file */
