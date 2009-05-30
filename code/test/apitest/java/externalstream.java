// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


import java.io.FileOutputStream;
import java.io.IOException;
import java.lang.RuntimeException;

import com.jagpdf.jagpdf;
import com.jagpdf.Document;
import com.jagpdf.Canvas;
import com.jagpdf.StreamOut;


//
//
//
class CustomStream extends StreamOut
{
    FileOutputStream m_file;

    public CustomStream(String pathname) {
        try {
            m_file = new FileOutputStream(pathname);
        }
        catch(IOException e) {
            throw new RuntimeException(e);
        }
    }

    public void write(byte[] data_in) {
        try {
            m_file.write(data_in);
        }
        catch(IOException e) {
            //throw e;
        }
    }

    public void close() {
        try {
            m_file.close();
        }
        catch(IOException e) {
            //throw e;
        }
    }
}

//
//
//
public class externalstream {
    //
    //
    //
    public static void main(String argv[]) {
        for(int i=0; i<1/*10000*/; ++i)
        {
            do_it(argv);
        }
        //collect(5000, "done");
    }

    public static void do_it(String argv[]) {
        CustomStream my_stream = new CustomStream(argv[0] + "/basic_extstream.pdf");
        Document doc = jagpdf.create_stream(my_stream);
        my_stream = null;
        doc.page_start(5.9*72, 3.5*72);
        doc.page_end();
        doc.finalize_doc();
        doc = null;
    }

    public static void collect(int ms, String s)
    {
        System.out.println(s + ": collecting ...");
        try {
            System.gc();
            Thread.sleep(ms);
        }
        catch(InterruptedException e) {
        }
        System.out.println(" done");

    }


//     public static void tests() {
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
//     }
}
