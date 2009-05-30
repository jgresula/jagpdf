// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "testcommon.h"
using namespace jag;

namespace
{
  EasyFontTTF g_font;

  void do_some(pdf::Document& doc)
  {
      doc.page_start(5.9*72, 5.9*72);
      pdf::Canvas page(doc.page().canvas());
      page.color("fs", 0.75);
      page.rectangle(20, 20, 72, 72);
      page.path_paint("s");
      page.text_font(g_font(8));
      page.text(25, 57, "a gray rectangle");
      page.text(25, 45, "www.boost.org");
      doc.page().annotation_uri(20, 20, 72, 72, "http://www.boost.org", 0);
      doc.page_end();
  }


  void do_goto_single_page(pdf::Document& doc)
  {
      pdf::Double pheight = 5.9*72;
      doc.page_start(5.9*72, pheight);
      pdf::Canvas page(doc.page().canvas());
      page.color("fs", 0.75);
      page.rectangle(20, 20, 72, 72);
      page.rectangle(20, pheight-92, 72, 72)    ;
      page.path_paint("s");
      page.text_font(g_font(8));
      page.text(25, 57, "click to zoom");
      page.text(25, 45, "to 200%");
      page.text(25, pheight-47, "click to fit height");
      doc.page().annotation_goto(20, pheight-92, 72, 72,
                                  StrFmt() << "page=" << doc.page_number() << ";mode=FitV", 0);
      doc.page().annotation_goto(20, 20, 72, 72,
                                  StrFmt() << "page=" << doc.page_number() << ";mode=XYZ;left=0;top=" << pheight << ";zoom=2", 0);
      doc.page_end();
  }


  pdf::Canvas page_vis(pdf::Document& doc, pdf::Int this_pagenr, pdf::Int other_pagenr)
  {
      pdf::Canvas page(doc.page().canvas());
      page.text_font(g_font(8));
      page.color("fs", 0.75);
      page.rectangle(20, 20, 72, 72);
      page.path_paint("s");
      page.text(25, 57, StrFmt() << "This is Page *" << this_pagenr << "*");
      page.text(25, 45, StrFmt() << "Click for page " << other_pagenr);
      return page;
  }


  void do_goto_each_other(pdf::Document& doc)
  {
      pdf::Destination dest_1st = doc.destination_reserve();
      pdf::Int page_nr = doc.page_number();
      doc.page_start(5.9*72, 5.9*72);
      pdf::Destination dest_1st_direct = doc.destination_define("mode=XYZ");
      pdf::Destination dest_2nd = doc.destination_reserve();

      pdf::Canvas page(page_vis(doc, page_nr+1, page_nr+2));
      doc.page().annotation_goto(20, 20, 72, 72, dest_2nd, 0);
      doc.page_end();

      doc.page_start(5.9*72, 5.9*72);
      page = page_vis(doc, page_nr+2, page_nr+1);
      doc.page().annotation_goto(20, 20, 72, 72, dest_1st, 0);
      doc.page_end();

      doc.page_start(5.9*72, 5.9*72);
      page = page_vis(doc, page_nr+3, page_nr+1);
      doc.page().annotation_goto(20, 20, 72, 72, dest_1st_direct, 0);
      doc.page_end();

      doc.destination_define_reserved(dest_1st, StrFmt() << "page=" << page_nr << ";mode=XYZ");
      doc.destination_define_reserved(dest_2nd, StrFmt() << "page=" << page_nr+1 << ";mode=XYZ");
  }


  void make_goto(
      pdf::Document& doc, pdf::Canvas& page, pdf::Double x, pdf::Double y,
      pdf::Destination dest, pdf::Char const* txt)
  {
      page.color("fs", 0.75);
      page.rectangle(x, y, 36, 36);
      page.path_paint("s");
      page.text(x+10, y+10, txt);
      doc.page().annotation_goto(x, y, 36, 36, dest, 0);
  }


  void do_outline_and_goto(pdf::Document& doc)
  {
      pdf::Int page_nr = doc.page_number();
      pdf::Destination p25 = doc.destination_define("mode=XYZ;zoom=0.25");
      doc.page_start(5.9*72, 5.9*72);
      pdf::Canvas page(doc.page().canvas());
      pdf::Destination p50 = doc.destination_define("mode=XYZ;zoom=0.50");
      pdf::Destination p75 = doc.destination_reserve();

      doc.outline().item(StrFmt() << "Page " << page_nr+1);
      doc.outline().level_down();
      doc.outline().item("zoom 25%", p25);
      doc.outline().item("zoom 50%", p50);
      doc.outline().item("zoom 75%", p75);
      doc.outline().level_up();
      page.text_font(g_font(8));
      make_goto(doc, page, 20, 20, p25, "25%");
      make_goto(doc, page, 20, 60, p50, "50%");
      make_goto(doc, page, 20, 100, p75, "75%");
      doc.page_end();
      doc.destination_define_reserved(p75, StrFmt() << "page=" << page_nr << ";mode=XYZ;zoom=0.75");
  }



  void test_main(int argc, char** argv)
  {
      register_command_line(argc, argv);

      pdf::Profile cfg = pdf::create_profile();
      cfg.set("doc.compressed", "0");
      pdf::Document doc(create_doc("annots.pdf", &cfg));
      g_font.set_writer(doc);

      do_some(doc);
      do_goto_single_page(doc);
      do_goto_each_other(doc);
      do_outline_and_goto(doc);

      doc.finalize();
  }
} //anonymous namespace

//
//
//
int annots(int argc, char ** const argv)
{
    return test_runner(test_main, argc, argv);
}



/** EOF @file */
