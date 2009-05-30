// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "canvasimpl.h"
#include "docwriterimpl.h"
#include "contentstream.h"
#include "pdffont.h"
#include "fontdictionary.h"
#include "interfaces/fontadapter.h"
#include <core/errlib/errlib.h>
#include <core/errlib/msg_writer.h>
#include <core/generic/scopeguard.h>
#include <core/jstd/unicode.h>
#include <core/generic/stringutils.h>
#include <core/jstd/icumain.h>
#include <interfaces/configinternal.h>
#include <interfaces/execcontext.h>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/scoped_ptr.hpp>
#include <core/generic/checked_cast.h>
#include <pdflib/cfgsymbols.h>

using namespace jag::jstd;
using namespace boost;

namespace jag {
namespace pdf {

namespace
{
  struct no_action_t { void operator()() const {} };

  // Defines a text sequence offsets.
  struct offsets_info_t
  {
      Double const* adj;   // array of offsets
      Int const*    pos;   // array of indices into a text
      Int           len;   // lenght of both arrays
  };

  //
  // Function object that is initialized with a sequence and a writer; is able
  // to output a slice of the sequence. Useful when the sequence items must be
  // interleaved with some other data (e.g. glyph offsets).
  //
  template<class T, class O>
  class advanced_string_writer
  {
      T const*  m_data;
      O         m_writer;
  public:
      advanced_string_writer(T const* data, O const& writer)
          : m_data(data)
          , m_writer(writer)
      {}

      void operator()(Long first, Long last) const
      {
          JAG_PRECONDITION(first<=last);
          if (first!=last)
              m_writer(m_data+first, last-first);
      }
  };

  //
  // Convenient helper that instantiates advanced_string_writer
  //
  template<class T, class O>
  advanced_string_writer<T,O> make_text_writer(T const* data, O const& writer)
  {
      return advanced_string_writer<T,O>(data, writer);
  }


  //
  // Writes a string interleaved with offsets.
  //
  template<class SW>
  void output_text_with_adjustments(SW const& string_writer,
                                     ObjFmtBasic& writer,
                                     Long str_len,
                                     offsets_info_t const& offsets)
  {
      writer.array_start();
      int last_written_char = 0;
      for(int i=0; i<offsets.len; ++ i)
      {
          string_writer(last_written_char, offsets.pos[i]);
          last_written_char = offsets.pos[i];
          writer.space().output(offsets.adj[i]).space();
      }
      string_writer(last_written_char, str_len);
      writer.array_end().graphics_op(OP_TJ);
  }


  ///
  /// Outputs a text written in a simple font.
  ///
  /// If the text has a different encoding (txt_enc) than the font the text is
  /// re-encoded. Also outputs glyph offsets (if any supplied).
  ///
  void output_simple_font_text(PDFFont const&font,
                                ObjFmtBasic& writer,
                                Char const* start, Char const* end,
                                offsets_info_t const* offsets,
                                char const* txt_enc)
  {
      std::vector<Char> chars;

      if (!is_empty(txt_enc))
      {
          // text encoding --> font encoding if they are different
          char const* txt_enc_canon = get_canonical_converter_name(txt_enc);
          if (strcmp(txt_enc_canon, font.font()->encoding_canonical()))
          {
              std::vector<UChar> uchars;
              to_unicode(txt_enc, start, end, uchars);
              JAG_ASSERT(! uchars.empty());
              from_unicode(font.font()->encoding_canonical(),
                           &uchars[0], &uchars[0]+uchars.size(), chars);

              start=&chars[0];
              end=start+chars.size();
          }
      }

      font.font_dict().use_char8(start, end);
      if (!offsets)
      {
          writer.text_string(start, end-start, false).graphics_op(OP_Tj);
      }
      else
      {
          typedef ObjFmtBasic& (ObjFmtBasic::*my_overload_t)(Char const*, size_t, bool);
          my_overload_t fn = &ObjFmtBasic::text_string;

          output_text_with_adjustments(
              make_text_writer(start,
                                boost::bind(fn,
                                            boost::ref(writer), _1, _2, false)),
              writer,
              end-start,
              *offsets);
      }
  }


  ///
  /// Outputs a text written in a composite font.
  ///
  /// If the text has a different encoding (txt_enc) than the font the text is
  /// re-encoded. Also outputs glyph offsets (if any supplied).
  ///
  void output_cid_font_text(PDFFont const&font,
                             ObjFmtBasic& writer,
                             Char const* start, Char const* end,
                             offsets_info_t const* offsets,
                             char const* txt_enc)
  {
      // convert the input to unicode codepoints
      std::vector<Int> codepoints;
      codepoints.reserve(end-start);

      if (is_empty(txt_enc))
      {
          UnicodeConverter* cnv(font.acquire_converter());
          JAG_ASSERT(cnv);
          while(start!=end)
              codepoints.push_back(cnv->next_code_point(&start, end));

          ON_BLOCK_EXIT_OBJ(font, &PDFFont::release_converter);
      }
      else
      {
          // Note: the converter is instantiated *always* even though it can be
          // the same as the one provided by the font. However, cost of finding
          // out whether they are the same is questionable as the encoding needs
          // to be converted to a canonical form and string compared.
          UnicodeConverter cnv(txt_enc);
          while(start!=end)
              codepoints.push_back(cnv.next_code_point(&start, end));
      }


      // send codepoints to font dictionary and in turn retrieve
      // a corresponding sequence of gids which are sent to output
      std::vector<Int> gids;
      font.font_dict().use_codepoints(&codepoints[0],
                                       &codepoints[0] + codepoints.size(),
                                       gids);
      if (!offsets)
      {
          writer.string_object_2b(&gids[0], gids.size()).graphics_op(OP_Tj);
      }
      else
      {
          typedef ObjFmtBasic& (ObjFmtBasic::*my_overload_t)(Int const*, size_t);
          my_overload_t fn = &ObjFmtBasic::string_object_2b;
          output_text_with_adjustments(
              make_text_writer(&gids[0],
                               boost::bind(fn, boost::ref(writer), _1, _2)),
              writer,
              gids.size(),
              *offsets);
      }
  }


  ///
  /// Low-level entry point for text output.
  ///
  void output_text_internal(PDFFont const&font,
                             ObjFmtBasic& writer,
                             Char const* start, Char const* end,
                             offsets_info_t const* offsets,
                             IExecContext const& exec_ctx)
  {
      JAG_PRECONDITION(start<end);
      char const* txt_enc = get_default_text_encoding(exec_ctx);

      if (PDFFontData::SIMPLE_FONT == font.font_dict().fdict_data().font_type())
      {
          output_simple_font_text(font, writer, start, end, offsets, txt_enc);
      }
      else
      {
          if (ENC_IDENTITY == font.font_dict().fdict_data().font_encoding())
          {
              output_cid_font_text(font, writer, start, end, offsets, txt_enc);
          }
          else
          {
              // check this path - renegade
              JAG_TBD;
              // we have an encoding PDF understands to; so just forward
              // it to font dictionary and send it to output as such
              font.font_dict().use_cids(start, end);
              writer.string_object_2b(start, end).graphics_op(OP_Tj);
          }
      }
  }


  // writes text object start and translates to specified position
  void write_label_prologue(ObjFmtBasic& writer, Double x, Double y)
  {
    writer
        .graphics_op(OP_BT)
        .output(x).space().output(y).graphics_op(OP_Td)
    ;
  }


} // anonymous namespace




///
/// Generic text showing functionality called by all text showing interface
/// functions.
///
template<class PRE_ACTION, class POST_ACTION>
void CanvasImpl::text_show_generic(Char const* start, Char const* end,
                                    Double const* offsets, UInt offsets_length,
                                    Int const* positions, UInt positions_length,
                                    PRE_ACTION const& pre_action,
                                    POST_ACTION const& post_action)
{
    if (offsets_length != positions_length)
        throw exception_invalid_value(msg_invalid_argument()) << JAGLOC;

    if (start==end)
        return;

    // get the current font form the graphics state; if there is no
    // font set by client then used the default one
    PDFFont const* font = m_graphics_state.top().font();
    if (!font)
    {
        IFont* fontid = m_doc_writer.default_font();
        if (fontid) {
            text_font(fontid);
            font = m_graphics_state.top().font();
        }
    }

    // the font retrieval must allways succeed
    JAG_ASSERT(font);

    if (font->has_multiple_encondings())
    {
        pre_action();
        text_show_multienc_font(font, start, end,
                                offsets, positions, positions_length);
        post_action();
        return;
    }


    // add font to the current resource list and adjust gr state accordingly
    ensure_resource_list().add_font(m_graphics_state.top().font()->font_dict());
    commit_graphics_state();

    pre_action();

    offsets_info_t offsets_info;
    if (offsets_length)
    {
        offsets_info.adj = offsets;
        offsets_info.pos = positions;
        offsets_info.len = positions_length;
    }

    output_text_internal(*font,
                          m_fmt,
                          start, end,
                          offsets_length ? &offsets_info : 0,
                          m_doc_writer.exec_context());

    post_action();
}


//
// Handles multi-enconding fonts. Uses an iterator giving consistent strings and
// correspoding fonts and calls text_show_generic() for each consistent string.
//
void CanvasImpl::text_show_multienc_font(
    PDFFont const* font,
    Char const* start, Char const* end,
    Double const* offsets,
    Int const* positions, UInt pos_length)
{
    JAG_PRECONDITION(font->has_multiple_encondings());

    PDFFontIterator it(font->create_iterator(start, end));
    std::vector<Char> enc_str;

    Int const* curr_pos = positions;
    Int const* end_pos = curr_pos + pos_length;
    Int curr_txt_len = 0;
    std::vector<Int> vpositions;
    std::vector<Double> voffsets;
    Int const* ppositions = 0;
    Double const* poffsets = 0;
    size_t pos_span_len = 0;

    // iterate over consistent strings
    for(;;)
    {
        PDFFont const* fnt = it.next(enc_str);
        if (!fnt)
            break;

        text_font_internal(*fnt);
        const Int enc_str_len = enc_str.size();

        if (positions)
        {
            // If the positioning stuff is present we need to find spans
            // corresponding to the consistent string.
            Int const* found = std::upper_bound(curr_pos,
                                                end_pos,
                                                curr_txt_len + enc_str_len);
            pos_span_len = found - curr_pos;
            if (!curr_txt_len)
            {
                // the first consistent string, use the original data
                ppositions = positions;
                poffsets = offsets;
            }
            else
            {
                size_t ptr_offset = curr_pos - positions;
                voffsets.assign(offsets + ptr_offset,
                                offsets + ptr_offset + pos_span_len);
                vpositions.assign(curr_pos, curr_pos + pos_span_len);

                // adjust string positions to the current consistent string
                for(size_t i=0; i<pos_span_len; ++i)
                    vpositions[i] -= curr_txt_len;

                ppositions = &vpositions[0];
                poffsets = &voffsets[0];
            }

                curr_pos = found;
                curr_txt_len += enc_str_len;
        }

        text_show_generic(&enc_str[0],
                          &enc_str[0] + enc_str_len,
                          poffsets, pos_span_len,
                          ppositions, pos_span_len,
                          no_action_t(), no_action_t());
    }

    // set the multi-enc font as the current font
    text_font_internal(*font);
}



//////////////////////////////////////////////////////////////////////////
// TEXT LABEL
//////////////////////////////////////////////////////////////////////////

//
//
//
void CanvasImpl::text_simple_ro(Double x, Double y,
                                 Char const* start, Char const* end,
                                 Double const* offsets, UInt offsets_length,
                                 Int const* positions, UInt positions_length)
{
    // the generic text show function is called with a pre-action that writes
    // start of the text object and the offset; upon returning the text object
    // is closed
    text_show_generic(start, end,
                       offsets, offsets_length,
                       positions, positions_length,
                       boost::bind(write_label_prologue, boost::ref(m_fmt), x, y),
                       boost::bind(&ObjFmtBasic::graphics_op, boost::ref(m_fmt), OP_ET));
}

//
//
//
void CanvasImpl::text_simple(Double x, Double y, Char const* text)
{
    text_simple_ro(x, y, text, text+strlen(text), 0, 0, 0, 0);
}

//
//
//
void CanvasImpl::text_simple_r(Double x, Double y,
                                    Char const* start, Char const* end)
{
    text_simple_ro(x, y, start, end, 0, 0, 0, 0);
}

//
//
//
void CanvasImpl::text_simple_o(Double x, Double y,
                        Char const* txt_u,
                        Double const* offsets, UInt offsets_length,
                        Int const* positions, UInt positions_length)
{
    text_simple_ro(x, y,
                    txt_u, txt_u+strlen(txt_u),
                    offsets, offsets_length,
                    positions, positions_length);
}



//////////////////////////////////////////////////////////////////////////
// MULTILINE TEXT
//////////////////////////////////////////////////////////////////////////

//
//
//
void CanvasImpl::text_start(Double x, Double y)
{
    m_fmt.graphics_op(OP_BT);
    text_translate_line(x, y);
}

//
//
//
void CanvasImpl::text_end()
{
    m_fmt.graphics_op(OP_ET);
}


//
//
//
void CanvasImpl::text_ro(Char const* start, Char const* end,
                         Double const* offsets, UInt offsets_length,
                         Int const* positions, UInt positions_length)
{
    text_show_generic(start, end,
                      offsets, offsets_length,
                      positions, positions_length,
                      no_action_t(),
                      no_action_t());
}


//
//
//
void CanvasImpl::text(Char const* txt)
{
    text_ro(txt, txt+strlen(txt), 0, 0, 0, 0);
}

//
//
//
void CanvasImpl::text_o(Char const* txt,
                        Double const* adjustments, UInt adjustments_length,
                        Int const* positions, UInt positions_length)
{
    text_ro(txt, txt+strlen(txt),
            adjustments, adjustments_length,
            positions, positions_length);
}

//
//
//
void CanvasImpl::text_r(Char const* start, Char const* end)
{
    text_ro(start, end, 0, 0, 0, 0);
}




///////////////////////////////////////////////////////////////////////////
// Text positioning
///////////////////////////////////////////////////////////////////////////

//
//
//
void CanvasImpl::text_translate_line(Double x, Double y)
{
    m_fmt.output(x).space().output(y).graphics_op(OP_Td);
}




///////////////////////////////////////////////////////////////////////////
// Text state
///////////////////////////////////////////////////////////////////////////


//
//
//
void CanvasImpl::text_font(IFont* font)
{
    // issued IFont* objects are encapsulated by an adapter which provides
    // corresponding PDFFFont object
    IFontAdapter* f = checked_static_cast<IFontAdapter*>(font);
    text_font_internal(f->font());
}


//
//
//
void CanvasImpl::text_font_internal(PDFFont const& fnt)
{
    m_graphics_state.top().font(fnt);
    if (!fnt.has_multiple_encondings())
    {
        // add font to resource list to ensure it appears in the output stream,
        // otherwise it could happen then no text is going to be written by that
        // font and thus no font is outputted
        if (!fnt.has_multiple_encondings())
            ensure_resource_list().add_font(
                m_graphics_state.top().font()->font_dict());
    }
}


//
//
//
void CanvasImpl::text_character_spacing(Double spacing)
{
    m_fmt.output(spacing).graphics_op(OP_Tc);
}

//
//
//
void CanvasImpl::text_horizontal_scaling(Double scaling)
{
    m_fmt.output(scaling).graphics_op(OP_Tz);
}


//
//
//
void CanvasImpl::text_rendering_mode(Char const* mode)
{
    enum {
        FILL =        1 << 0,
        STROKE =      1 << 1,
        CLIP =        1 << 2,
        INVISIBLE =   1 << 3,
    };

    unsigned op = 0;
    for (;*mode; ++mode)
    {
        switch(*mode)
        {
        case 'f':
        case 'F':
            op |= FILL;
            break;
        case 's':
        case 'S':
            op |= STROKE;
            break;
        case 'c':
        case 'C':
            op |= CLIP;
            break;
        case 'i':
        case 'I':
            op |= INVISIBLE;
            break;

        default:
            throw exception_invalid_value(msg_invalid_argument()) << JAGLOC;
        }
    }

    int operand;
    switch(op)
    {
    case FILL:              operand = 0; break;
    case STROKE:            operand = 1; break;
    case FILL|STROKE:       operand = 2; break;
    case INVISIBLE:         operand = 3; break;
    case FILL|CLIP:         operand = 4; break;
    case STROKE|CLIP:       operand = 5; break;
    case FILL|STROKE|CLIP:  operand = 6; break;
    case CLIP:              operand = 7; break;
        default:
            throw exception_invalid_value(msg_invalid_argument()) << JAGLOC;
    }

    m_fmt.output(operand).graphics_op(OP_Tr);
}

//
//
//
void CanvasImpl::text_rise(Double rise)
{
    m_fmt.output(rise).graphics_op(OP_Ts);
}




/*
TO BE DONE - seems outdated

- store GID -> Unicode mapping somewhere so that it can
  be later picked up by CMap

- store somewhere information forcing a font descriptor
  to embed its font program

- sharing font descriptor, figure out some strategy; simple
  font and composite font can share a font descriptor, but
  font descriptor dict can have extra keys for composite fonts
*/



}} //namespace jag::pdf



