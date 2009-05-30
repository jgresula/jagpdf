/*
 * Copyright (c) 2005-2009 Jaroslav Gresula
 *
 * Distributed under the MIT license (See accompanying file
 * LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
 *

 */
#include "ctestcommon.h"
#include <assert.h>

static
jag_Document pdf_file_proxy(char **argv, char const* fname, jag_Profile cfg)
{
    char fpath[1024];
    sprintf(fpath, "%s/jagpdf_doc_%s", argv[1], fname);
    return jag_create_file(fpath, cfg);
}

/* FWD */
static
void test_main(int argc, char** argv);


/* ----------------------------------------------------------------------
                        Error Handling

 */
void error_handling()
{
    jag_Canvas hc=0;
    jag_Document doc=0;
    jag_Profile profile=0;
    assert(!"do not run, just compile");

    /*[c_example_error_handling */
    /*` The majority of [lib] functions returns an error code -
      `jag_error`. Value 0 indicates that the operation succeeded, non-zero
      error code signalizes an error. */
    if (jag_Canvas_line_to(hc, 10.0, 10.0)) {
        /* process the error */
    }

    /*` Some functions return an object. If such function fails a ['null] object
      is returned. */
    doc = jag_create_file("/path/to/file/", profile);
    if (!doc) {
        /* process the error */
    }

    /*]*/
}




/* ----------------------------------------------------------------------
                        Custom Stream

 */

/*[c_example_custom_stream */
jag_Int JAG_CALLSPEC my_write(void* custom, void const* data, jag_ULong size)
{ /*<-*/ JAG_IGNORE_UNUSED(custom); JAG_IGNORE_UNUSED(data); JAG_IGNORE_UNUSED(size); /*->*/
    /* write data */
    return 0;
}

jag_Int JAG_CALLSPEC my_close(void* custom)
{ /*<-*/ JAG_IGNORE_UNUSED(custom); /*->*/
    /* finish */
    return 0;
}

void use_custom_stream(void* custom)
{ /*<-*/ jag_Document doc; jag_Profile profile=0;/*->*/
    /* ... */
    jag_streamout my_stream;
    my_stream.write = my_write;
    my_stream.close = my_close;
    my_stream.custom_data = (void*)custom;
    doc = jag_create_stream(&my_stream, profile);
    /* ... */
}
/*]*/




/* ----------------------------------------------------------------------
                        Hello world

 */
#define jag_create_file(fname, c) pdf_file_proxy(argv, fname, c)

/*[ c_example_hello_world */
#include <jagpdf/api.h>
/*<-*/
#define main helloworld
/*->*/
int main(int argc, char** argv)
{
    jag_Document doc;
    jag_Page page;
    jag_Canvas canvas; /*<-*/ test_main(argc, argv); /*run also other examples*/ /*->*/
    doc = jag_create_file("hello.pdf", 0);
    jag_Document_page_start(doc, 597.6, 848.68);
    page = jag_Document_page(doc);
    canvas = jag_Page_canvas(page);
    jag_Canvas_text_simple(canvas, 50, 800, "Hello, world!");
    jag_Document_page_end(doc);
    jag_Document_finalize(doc);
    jag_release(doc);
    return 0;
} /*]*/




static
void test_main(int argc, char** argv)
{
    JAG_IGNORE_UNUSED(argc);
    JAG_IGNORE_UNUSED(argv);
}



/** EOF @file */
