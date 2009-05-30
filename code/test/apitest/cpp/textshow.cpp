// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


// tests text showing methods

#include "testcommon.h"

using namespace jag;

namespace
{
  EasyFont g_font;

  class TextObject
  {
      pdf::Canvas m_cnv;

  public:
      TextObject(pdf::Canvas cnv, pdf::Double x, pdf::Double y)
          : m_cnv(cnv)
      {
          m_cnv.text_start(x, y);
      }

      ~TextObject()
      {
          m_cnv.text_end();
      }
  };


//
//
//
  void multiline(pdf::Document& doc)
  {
      doc.page_start(140, 160);
      pdf::Canvas cnv(doc.page().canvas());
      cnv.text_font(g_font(10));
      pdf::Double linesp = g_font(10).height();

      char const* l1 = "this is line #1";
      char const* l1_end = l1 + strlen(l1);
      char const* l2 = "this is line #2";
      char const* l2_end = l2 + strlen(l2);

      pdf::Double offsets[2] = { -1000.0, -200.0 };
      pdf::Int    pos[2]     = { 4, 8 };

      {
          TextObject txt(cnv, 20, 20);
          cnv.text(l1);
          cnv.text_translate_line(0, linesp);
          cnv.text(l2);
      }

      {
          TextObject txt(cnv, 20, 50);
          cnv.text(l1, l1_end);
          cnv.text_translate_line(0, linesp);
          cnv.text(l2, l2_end);
      }

      {
          TextObject txt(cnv, 20, 80);
          cnv.text(l1, offsets, 2, pos, 2);
          cnv.text_translate_line(0, linesp);
          cnv.text(l2, offsets, 2, pos, 2);
      }

      {
          TextObject txt(cnv, 20, 110);
          cnv.text(l1, l1_end, offsets, 2, pos, 2);
          cnv.text_translate_line(0, linesp);
          cnv.text(l2, l2_end, offsets, 2, pos, 2);
      }



      doc.page_end();
  }


//
//
//
  void labels(pdf::Document& doc)
  {
      doc.page_start(200, 100);
      pdf::Canvas cnv(doc.page().canvas());
      cnv.text_font(g_font(10));

      cnv.text(20, 20, "classic label");

      char const* s1 = "label as a range";
      cnv.text(20, 40, s1, s1+strlen(s1));

      pdf::Double offsets[1] = { -1000.0 };
      pdf::Int    pos[1] = { 5 };
      char const* s2 = "label with offsets";
      cnv.text(20, 60, s2, offsets, 1, pos, 1);

      char const* s3 = "label with offsets - range";
      cnv.text(20, 80, s3, s3+strlen(s3), offsets, 1, pos, 1);

      doc.page_end();
  }


//
//
//
  void test_main(int argc, char** argv)
  {
      register_command_line(argc, argv);
      pdf::Profile cfg = pdf::create_profile();
      cfg.set("doc.compressed", "0");
      pdf::Document doc(create_doc("textshow.pdf", &cfg));
      g_font.set_writer(doc);

      labels(doc);
      multiline(doc);

      doc.finalize();
  }

} //namespace


//
//
//
int textshow(int argc, char ** const argv)
{
    return test_runner(test_main, argc, argv);
}



/** EOF @file */
