// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include <stdio.h>
#include <jagpdf/api.h>
#include <string>
#include <iostream>

using namespace jag;

int basic(int, char ** const argv)
{
    try
    {
        std::string out_file(argv[1]);
        out_file += "/basic.pdf";
        pdf::Profile cfg(pdf::create_profile());
        cfg.set("doc.compressed", "0");
        pdf::Document doc(pdf::create_file(out_file.c_str(), pdf::Profile()));
        doc.page_start(5.9*72, 3.5*72);
        doc.page_end();
        doc.finalize();
    }
    catch(pdf::Exception const& exc)
    {
        std::cerr << "  ERROR(" << exc.code() << "): " << exc.what() << '\n';
        std::cerr << ">> cpp client failed\n";
        return 1;
    }

    return 0;
}



/** EOF @file */
