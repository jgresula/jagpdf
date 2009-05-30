// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


import com.jagpdf.jagpdf;
import com.jagpdf.Document;
import com.jagpdf.Canvas;

public class basic {
    //
    //
    //
    public static void main(String argv[]) {
            tests();
            Document doc = jagpdf.create_file(argv[0] + "/basic.pdf");
            doc.page_start(5.9*72, 3.5*72);
            doc.page_end();
            doc.finalize_doc();
    }

    public static void tests() {
        int major = jagpdf.this_version_major;
        int minor = jagpdf.this_version_minor;
        int patch = jagpdf.this_version_patch;
//         Document doc = jagpdf.create_file("c:/temp/j.pdf");
//         doc.page_start(5.9*72, 3.5*72);
//         Canvas canvas = doc.page().canvas();
//         canvas.line_width(8);
//         long[] phase = new long[2];
//         phase[0] = 2;
//         phase[1] = 3;
//         canvas.line_dash(phase, 2, 11);
//         canvas.move_to(50, 100);
//         canvas.line_to(300, 100);
//         canvas.path_paint("fs");
//         doc.page_end();
//         doc.finalize_doc();
    }
}
