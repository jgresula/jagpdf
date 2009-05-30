// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include <string>
#include "testcommon.h"

#if (defined _MSC_VER)
# pragma warning(disable : 4702) //C4702: unreachable code
#endif


//fwd
void example_main(int argc, char** argv);

// ------------------------------------------------------------------------
//                      Hello world
//
//[ cpp_example_hello_world
#include <jagpdf/api.h>
using namespace jag;

//<-
#define main helloworld
//->
int main(int argc, char** const argv)
{
    //<-
    /* //->
    pdf::Document doc(pdf::create_file("hello.pdf"));
    //<- */
    example_main(argc, argv);  // run other examples in this file
    std::string fname(argv[1]);
    fname += "/jagpdf_doc_hello.pdf";
    pdf::Document doc(pdf::create_file(fname.c_str()));
    //->
    doc.page_start(597.6, 848.68);
    doc.page().canvas().text(50, 800, "Hello, world!");
    doc.page_end();
    doc.finalize();
    return 0;
}
//]


// ------------------------------------------------------------------------
//                      Error handling
//
void error_handling()
{
    //[cpp_example_error_handling
    try {
        // jagpdf usage
    }
    catch(pdf::Exception const& exc) {
        exc.what();  // human readable message
        exc.code();  // associated error code
    }
    //]
}


// ------------------------------------------------------------------------
//                      Custom stream
//
void custom_stream()
{
    //[cpp_example_custom_stream
    class MyStream
        : public pdf::StreamOut
    {
        pdf::Int write(void const* data, pdf::ULong size) { /*<-*/ignore_unused(data); ignore_unused(size);/*->*/
            // write data
            return 0;
        }

        pdf::Int close() {
            // finish
            return 0;
        }
    };

    MyStream stream;
    pdf::Document doc(pdf::create_stream(&stream));
    //]
}




void example_main(int argc, char** argv)
{
    ignore_unused(argc);
    ignore_unused(argv);

    error_handling();
    custom_stream();
}




/** EOF @file */
