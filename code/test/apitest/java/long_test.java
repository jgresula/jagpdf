// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


import java.io.FileOutputStream;
import java.io.IOException;
import java.lang.RuntimeException;
import java.io.ByteArrayOutputStream;

import com.jagpdf.jagpdf;
import com.jagpdf.Document;
import com.jagpdf.Canvas;
import com.jagpdf.Image;
import com.jagpdf.StreamOut;
import com.jagpdf.Profile;

import testlib.testlib;


//
//
//
class MemoryStream extends StreamOut
{
    public ByteArrayOutputStream m_out = new ByteArrayOutputStream();

    public void write(byte[] data_in)
    {
        try
        {
            m_out.write(data_in);
        }
        catch(IOException ignore) {}
    }

    public void close()
    {
        try
        {
            m_out.close();
        }
        catch(IOException ignore) {}
    }
}


//
//
//
public class long_test {
    static MemoryStream s_stream = new MemoryStream();
    static String s_profile =
        "info.title = title\n" +
        "info.author = unknown\n" +
        "info.subject = unknown\n" +
        "info.keywords = unknown\n" +
        "info.creator = unknown\n" +
        "info.creation_date = unknown\n";


    public static void main(String argv[]) {
        // initial document
        //do_it(s_stream);
        do_it(null);

        // thread stuff
        int num_threads = 10;
        int docs_per_thread = 1;

        while(true)
        {
            Thread[] threads = new Thread[num_threads];
            for(int i=0; i<num_threads; ++i) {
                threads[i] = new WorkerThread(docs_per_thread);
                threads[i].start();
            }

            for(int i=0; i<num_threads; ++i) {
                try {
                    threads[i].join();
                } catch(InterruptedException ignore) {}
            }
        }

//         for(int i=0; i<1000000; ++i)
//         {
//             do_it();
//         }
//         //collect(5000, "done");
    }

    public static void do_it(MemoryStream out_stream)
    {
        MemoryStream my_stream;
        if (out_stream == null)
        {
            my_stream = new MemoryStream();
        }
        else
        {
            my_stream = out_stream;
        }

        jagpdf.create_profile_from_string(s_profile);

        Document doc = jagpdf.create_stream(my_stream);
        doc.page_start(5.9*72, 3.5*72);
        Canvas canvas = doc.page().canvas();
        // meat
        String res_dir = testlib.getResourcesDir();
        String image_path = res_dir + "/images-jpeg/PalmTree-CMYK-icc-FOGRA27.jpg";
        Image img = doc.image_load_file(image_path);
        canvas.image(img, 50, 50);
        canvas.text(10, 10,
                    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" +
                    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" +
                    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" +
                    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" +
                    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" +
                    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" +
                    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" +
                    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" +
                    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" +
                    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" +
                    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" +
                    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" +
                    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" +
                    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" +
                    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" +
                    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" +
                    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" +
                    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" +
                    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" +
                    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
        // -
        doc.page_end();
        doc.finalize_doc();
        doc = null;

//         if (out_stream == null)
//         {
//             // compare the the doc is ok
//             assert my_stream.m_out.size() > 0;
//             assert my_stream.m_out.toByteArray() == s_stream.m_out.toByteArray();
//         }

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
}

//
//
//
class WorkerThread extends Thread {
    int m_count;
    WorkerThread(int count) {
        m_count = count;
    }

    public void run() {
        for(int i=0; i<m_count; ++i) {
            long_test.do_it(null);
        }
    }
}
