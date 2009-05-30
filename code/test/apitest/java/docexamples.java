// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


import com.jagpdf.jagpdf;
import com.jagpdf.Document;
import com.jagpdf.Canvas;
import com.jagpdf.Font;
import com.jagpdf.JagPDFException;
import com.jagpdf.StreamOut;

import testlib.testlib;


public class docexamples
{
    public static void main(String[] args)
    {
        strings(args);
    }


//
// The only purpose of the following code fragments is to be included by the
// documentation.
//

// ---------------------------------------------------------------------------
//             strings

    static void strings(String[] args)
    {
        Document doc = jagpdf.create_file(args[0] + "/jagpdf_doc_java_strings.pdf");
        doc.page_start(597.6, 848.68);
        Canvas canvas = doc.page().canvas();

        //[java_example_string
        /*` An example should make it clear. Let's load a standard ISO-8859-2
            encoded font. */
        Font helv = doc.font_load("standard; enc=iso-8859-2; name=Helvetica; size=24");
        canvas.text_font(helv);
        /*` One of the languages representable by this encoding is Czech. We can
          pass a Unicode string ['úplněk] (full moon). [lib] converts the string
          to ISO-8859-2 and it is shown correctly. */
        String full_moon_cze = "\u00fapln\u011bk";
        canvas.text(50, 800, full_moon_cze); // ok
        /*` If we pass Swedish ['fullmåne], letter ['å] will not be shown
         because ISO-8859-2 does not represent such character. We should have
         used ISO-8859-1 encoded font instead. */
        String full_moon_swe = "fullm\u00e5ne";
        canvas.text(50, 760, full_moon_swe); // wrong
        /*` Let's load a Unicode encoded TrueType font. */
        //<-
        /* //->
        Font dejavu = doc.font_load("enc=utf-8; file=DejaVuSans.ttf; size=24");
        //<- */
        String res_dir = testlib.getResourcesDir();
        String dejavu_file = res_dir + "/fonts/DejaVuSans.ttf";
        Font dejavu = doc.font_load("enc=utf-8; file=" + dejavu_file + "; size=24");
        //->
        canvas.text_font(dejavu);
        /*` Now we can mix Czech and Swedish and it will be shown correctly as
            [lib] converts the strings to UTF-8.*/
        canvas.text(50, 720, full_moon_swe);
        canvas.text(50, 680, full_moon_cze);
        //]

        doc.page_end();
        doc.finalize_doc();
    }
}



// ---------------------------------------------------------------------------
//             custom streams


//[java_example_custom_stream
class MyStream extends StreamOut
{
    public void write(byte[] data_in) {
        // write data
    }

    public void close() {
        // finish
    }
}
//<-
class JustToShowSomeCode
{
    public static void some_code() {
//->

MyStream myStream = new MyStream();
Document doc = jagpdf.create_stream(myStream);
//<-
    }
}
//->
//]

// ---------------------------------------------------------------------------
//             error handling


class Bogus
{
    public static void some_code() {
//[java_example_error_handling
try {
    // JagPDF usage
} catch(JagPDFException exc) {
    // process the exception
}
//]
    }
}
