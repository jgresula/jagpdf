// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include <cstdio>
#include <string>
#include <iostream>

#include "testcommon.h"
using namespace jag;

namespace
{
  EasyFontTTF g_font;
  pdf::Double g_page_height = 7*72;
  enum { ITALIC=1, BOLD=2 };

  struct rgb_color
  {
      rgb_color(pdf::Double r=-1.0, pdf::Double g=-1.0, pdf::Double b=-1.0)
          : red(r), green(g), blue(b)
      {}

      pdf::Double red;
      pdf::Double green;
      pdf::Double blue;
  };

  struct coord
  {
      coord(pdf::Double x_=-1.0, pdf::Double y_=-1.0)
          : x(x_), y(y_) {}

      pdf::Double x;
      pdf::Double y;
  };


  static
  void do_page(pdf::Document& doc,
                pdf::Char const* title,
                pdf::Int style = -1,
                rgb_color color = (rgb_color()),
                coord pos = coord(),
                pdf::Char const* text = 0)
  {
      pdf::DocumentOutline outline(doc.outline());
      if (!text)
          text = title;
      doc.page_start(5.9*72, g_page_height);
      if (style > 0 && doc.version() >= 4)
          outline.style(style);
      if (color.red >= 0.0 && doc.version() >= 4)
          outline.color(color.red, color.green, color.blue);

      if (pos.x < 0.0) {
          outline.item(title);
      } else {
          std::ostringstream ss;
          ss << "mode=XYZ; left=" << pos.x << ";top=" << pos.y;
          outline.item(title, ss.str().c_str());
      }

      pdf::Canvas page(doc.page().canvas());
      page.text_font(g_font());
      page.text(20, g_page_height/2, text);
      doc.page_end();
      g_page_height -= 36;
  }

  static
  void standard_usage(pdf::Document& doc)
  {
      pdf::DocumentOutline outline(doc.outline());
      do_page(doc, "P\xc5\x99\xc3\xadli\xc5\xa1 \xc5\xbelu\xc5\xa5ou\xc4\x8dk\xc3\xbd k\xc5\xaf\xc5\x88", -1, rgb_color(), coord(), "utf16-be bookmark");
      outline.state_save();
      do_page(doc, "2nd page - gray", 0, rgb_color(0.5,0.5,0.5));
      outline.state_restore();
      do_page(doc, "3rd page - default style");
      outline.level_down();
      outline.state_save();
      do_page(doc, "3a page - bold red", BOLD, rgb_color(1.0,0.0,0.0));
      outline.state_save();
      do_page(doc, "3b page - 1/2h bold italic red", BOLD|ITALIC, rgb_color(), coord(0, g_page_height/2));
      outline.state_restore();
      outline.level_down();
      do_page(doc, "3b1 page - bold red");
      outline.level_up();
      outline.state_restore();
      do_page(doc, "3c page - default style");
      // the last level_up() is not called intentionally as the
      // implementation is supposed to take care about that
  }


  static
  void do_document(pdf::Profile& cfg, char const* name)
  {
      pdf::Document doc(create_doc(name, &cfg));
      g_font.set_writer(doc);
      standard_usage(doc);
      doc.finalize();
  }


  static
  void fault_injection(char const* name)
  {
      pdf::Document doc(create_doc(name));
      pdf::DocumentOutline outline(doc.outline());

      JAG_MUST_THROW(outline.item("Invalid bookmark"))
          outline.state_save();
      doc.page_start(5.9*72, g_page_height);
      JAG_MUST_THROW(outline.level_up())
          doc.page_end();
      outline.state_restore();
      JAG_MUST_THROW(outline.state_restore())
          doc.finalize();
  }

  static
  void do_invalid_destinations(pdf::Profile& cfg, char const* name)
  {
      pdf::Char const* invalid_dests[] = {
          "zoom=1.2",
          "mode=nonsense",
          "mode=XYZ;zoom=onan",
          "mode=FitR;left=1;top=1;bottom=1",
          0 };

      for(pdf::Char const** d=invalid_dests; *d; ++d)
      {
          pdf::Document doc(create_doc(name, &cfg));
          doc.page_start(10.0*72, 10.0*72);
          doc.outline().item("~", *d);
          doc.page_end();
          JAG_MUST_THROW(doc.finalize())
              }

      pdf::Char const* syntax_err_dests[] = { "oom=1.2", 0 };

      pdf::Document doc(create_doc(name, &cfg));
      doc.page_start(10.0*72, 10.0*72);
      for(pdf::Char const** d1=syntax_err_dests; *d1; ++d1)
      {
          JAG_MUST_THROW(doc.outline().item("~", *d1))
              }
      doc.page_end();
      doc.finalize();
  }




  static
  void do_generic_bookmarks(pdf::Profile& cfg, char const* name)
  {
      const pdf::Double rl = 72;
      const pdf::Double rt = 9*72;
      const pdf::Double rr = 72+144;

      pdf::Document doc(create_doc(name, &cfg));
      pdf::DocumentOutline outline(doc.outline());

      doc.page_start(10.0*72, 10.0*72);
      pdf::Canvas page(doc.page().canvas());
      page.rectangle(rl, 7*72, 144, 144);
      page.path_paint("s");
      page.rectangle(72+36, 7*72+36, 72, 72);
      page.path_paint("s");

      outline.item("Zoom 100%", "mode=XYZ;zoom=1.0");
      outline.item("Zoom 250%", "mode=XYZ;zoom=2.5");
      outline.item("Zoom 25%", "mode=XYZ;zoom=.25");

      outline.item("Rect top-left, retain zoom",
                    StrFmt() << "mode=XYZ;left=" << rl << ";top=" << rt);

      outline.item("Fit width, position rectangle top",
                    StrFmt() << "mode=FitH;top=" << rt);

      outline.item("Fit width, retain y", "mode=FitH");
      outline.item("Fit height, position rectangle right",
                    StrFmt() << "mode=FitV;left=" << rr);

      outline.item("Fit height, retain x", "mode=FitV");
      outline.item("Fit inner rectangle", "mode=FitR;left=108;top=612;bottom=540;right=180");


      outline.item("Fit page", "mode=Fit");
      outline.item("Fit page bbox", "mode=FitB");
      outline.item("Fit bbox width, retain y", "mode=FitBH");

      outline.item("Fit bbox width, top 1/2 rect", StrFmt() << "mode=FitBH;top=" << rt-72);
      outline.item("Fit bbox height, retain x", "mode=FitBV");

      outline.item("Fit bbox height, left 1/2 rect", StrFmt() << "mode=FitBV;left=" << rl+72);
      outline.item(0, "mode=XYZ;zoom=1.5");
      outline.item("^^ an empty bookmark that zooms to  150%", "mode=XYZ;zoom=1.0");
      doc.page_end();
      doc.finalize();
  }



  void test_main(int argc, char** argv)
  {
      register_command_line(argc, argv);

      pdf::Profile cfg = pdf::create_profile();
      cfg.set("doc.compressed", "0");

      cfg.set("doc.trace_level", "5");

      do_invalid_destinations(cfg, "docoutline_invalid_dest.pdf");
      do_generic_bookmarks(cfg, "docoutline_generic.pdf");

      do_document(cfg, "docoutline.pdf");
      cfg.set("doc.version", "3");
      do_document(cfg, "docoutline13.pdf");

      cfg.set("doc.version", "4");
      cfg.set("doc.encryption", "standard");
      cfg.set("info.static_producer", "1");
      cfg.set("doc.static_file_id", "1");
      cfg.set("info.creation_date", "0");

      do_document(cfg, "docoutline_enc.pdf");

      fault_injection("docoutline_failed.pdf");
  }
} // anonymous namespace

int docoutline(int argc, char ** const argv)
{
    return test_runner(test_main, argc, argv);
}



/** EOF @file */
