// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


//[ java_example_hello_world
import com.jagpdf.jagpdf;
import com.jagpdf.Document;
import com.jagpdf.Font;
import com.jagpdf.Profile;
import com.jagpdf.Canvas;

import java.util.Arrays;
import java.util.ArrayList;
import java.util.List;

import testlib.testlib;


class FmtInfo
{
    public int[] positions;
    public double[] offsets;
}

public class basic_text {
        static String horse = "\u017elu\u0165ou\u010dk\u00fd k\u016f\u0148 \u00fap\u011bl";
        static String horse3 = horse + ' ' + horse + ' ' + horse;


    public static void main(String argv[]) {
        Document doc = jagpdf.create_file(argv[0] + "/basic_text.pdf");
        doc.page_start(597.6, 848.68);
        Canvas canvas = doc.page().canvas();
        doc.outline().item(horse);
        // adobe core font
        Font font_14 = doc.font_load("standard;name=Times-Roman;size=12;enc=windows-1250");
        canvas.text_font(font_14);
        canvas.text(50, 800, horse);
        // true type
        String res_dir = testlib.getResourcesDir();
        String fspec = "enc=utf-8; size=12; file=" + res_dir + "/fonts/DejaVuSans.ttf";
        Font font_ttf = doc.font_load(fspec);
        canvas.text_font(font_ttf);
        canvas.text(50, 750, horse);
        // control rect
        canvas.rectangle(50, 200, 400, 450);
        canvas.path_paint("s");
        // justification
        justified_text(font_ttf, canvas, 600);
        justified_text(font_14, canvas, 550);

        // expected errors
        try {
            int[] arr_i = new int[10];
            canvas.text(0, 0, arr_i);
            assert false;
        } catch(IllegalArgumentException expected) {}

        try {
            byte[] arr_b = new byte[10];
            canvas.text(0, 0, arr_b);
            assert false;
        } catch(IllegalArgumentException excpected) {}

        // finalize
        doc.page_end();
        doc.finalize_doc();
    }


    public static void justified_text(Font font, Canvas canvas, double y)
    {
        FmtInfo fmt_info = justify_string(font, horse3, 400.0);
        canvas.text_font(font);
        canvas.text_start(50, y);
        canvas.text(horse3,
                    fmt_info.offsets, fmt_info.offsets.length,
                    fmt_info.positions, fmt_info.positions.length);
        canvas.text_end();
    }


    public static FmtInfo justify_string(Font font, String str, double req_width)
    {
        double width = font.advance(str);
        int num_spaces = 0;
        FmtInfo result = new FmtInfo();
        List positions = new ArrayList();

        for(int i = 0; i < str.length(); ++i)
        {
            if (str.charAt(i) == ' ')
            {
                positions.add((Object)new Integer(i));
                ++num_spaces;
            }
        }

        double coeff = -1000.0 / font.size();
        double per_space = coeff * (req_width - font.advance(str)) / num_spaces;

        result.offsets = new double[num_spaces];
        result.positions = new int[positions.size()];
        Arrays.fill(result.offsets, per_space);

        for(int i=0; i<positions.size(); ++i)
        {
            result.positions[i] = ((Integer)positions.get(i)).intValue();
            System.out.println(result.positions[i]);
            System.out.println(result.offsets[i]);
        }



        return result;
    }
}
//]
