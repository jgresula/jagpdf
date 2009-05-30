// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


import com.jagpdf.jagpdf;
import com.jagpdf.Document;
import com.jagpdf.StreamOut;
import com.jagpdf.Font;
import com.jagpdf.Canvas;
import com.jagpdf.Image;
import com.jagpdf.Profile;

import testlib.testlib;


public class encrypted {
    static String horse = "\u017elu\u0165ou\u010dk\u00fd k\u016f\u0148 \u00fap\u011bl";

    public static void main(String argv[]) {
        Profile profile = jagpdf.create_profile();
        profile.set("doc.encryption", "standard");
        profile.set("doc.static_file_id", "1");
        profile.set("info.static_producer", "1");
        profile.set("info.creation_date", "0");
        profile.set("stdsh.pwd_user", "user");
        profile.set("stdsh.pwd_owner", "owner");
        Document doc = jagpdf.create_file(argv[0] + "/encrypted.pdf", profile);
        doc.page_start(597.6, 848.68);
        Canvas canvas = doc.page().canvas();
        String res_dir = testlib.getResourcesDir();
        String image_path = res_dir + "/images/lena.jpg";
        // bookmark
        doc.outline().item(horse);
        // image
        Image img = doc.image_load_file(image_path);
        canvas.image(img, 50, 50);
        // adobe core font
        Font font_14 = doc.font_load("standard;name=Times-Roman;size=12;enc=windows-1250");
        canvas.text_font(font_14);
        canvas.text(50, 800, horse);
        // true type
        String fspec = "enc=utf-8; size=12; file=" + res_dir + "/fonts/DejaVuSans.ttf";
        Font font_ttf = doc.font_load(fspec);
        canvas.text_font(font_ttf);
        canvas.text(50, 750, horse);
        // finalize
        doc.page_end();
        doc.finalize_doc();
    }
}
