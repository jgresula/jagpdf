// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


import com.jagpdf.jagpdf;
import com.jagpdf.Document;
import com.jagpdf.Image;
import com.jagpdf.Canvas;
import com.jagpdf.ImageFormat;
import com.jagpdf.ImageDef;
import com.jagpdf.ColorSpaceType;

import com.jagpdf.JagPDFException;
import testlib.testlib;

public class basic_image {
    public static void main(String argv[]) {
        Document doc = jagpdf.create_file(argv[0] + "/basic_image.pdf");
        doc.page_start(597.6, 848.68);
        Canvas canvas = doc.page().canvas();
        String res_dir = testlib.getResourcesDir();
        String image_path = res_dir + "/images/lena.jpg";
        //
        // image format autodetection
        //
        Image img = doc.image_load_file(image_path);
        canvas.image(img, 50, 50);
        //
        // image format specification (tests enums as well)
        //
        Image img_enum = doc.image_load_file(image_path,
                                             ImageFormat.IMAGE_FORMAT_JPEG);
        canvas.image(img_enum, 310, 50);
        //
        // custom image
        //
        int img_dim = 7;
        byte[] checker_bytes = new byte[img_dim * img_dim];
        for(int i = 0; i < img_dim * img_dim; ++i)
        {
            if (i % 2 == 0)
                checker_bytes[i] = 0;
            else
                checker_bytes[i] = (byte)255;
        }
        ImageDef imgdef = doc.image_definition();
        imgdef.data(checker_bytes, img_dim * img_dim);
        imgdef.dimensions(img_dim, img_dim);
        imgdef.color_space(ColorSpaceType.CS_DEVICE_GRAY);
        imgdef.bits_per_component(8);
        imgdef.dpi(9, 9);
        imgdef.format(ImageFormat.IMAGE_FORMAT_NATIVE);
        Image img_custom = doc.image_load(imgdef);
        canvas.image(img_custom, 50, 310);
        //
        // bad format specification (tests exceptions as well)
        //
        try
        {
            Image img_err = doc.image_load_file(image_path,
                                                ImageFormat.IMAGE_FORMAT_PNG);
            throw new RuntimeException("JagPDFException expected.");
        }
        catch(JagPDFException exc)
        {
            // ok, as expected
        }

        // finalize
        doc.page_end();
        doc.finalize_doc();
    }
}
//]
