// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


//[ java_example_hello_world
import com.jagpdf.jagpdf;
import com.jagpdf.Document;

public class helloworld {
    public static void main(String argv[]) {
        //<-
        /* //->
        Document doc = jagpdf.create_file("hello.pdf");
        //<- */
        Document doc = jagpdf.create_file(argv[0] + "/jagpdf_doc_hello.pdf");
        //->
        doc.page_start(597.6, 848.68);
        doc.page().canvas().text(50, 800, "Hello, world!");
        doc.page_end();
        doc.finalize_doc();
    }
}
//]




