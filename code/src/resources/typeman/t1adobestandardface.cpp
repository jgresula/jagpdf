// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "t1adobestandardface.h"
#include "fontspecimpl.h"
#include <interfaces/streams.h>
#include <core/errlib/errlib.h>
#include <msg_resources.h>
#include <string.h>

using namespace boost;

namespace jag {
namespace resources {

namespace
{
  struct t1_font_map {
      Char const*             name;
      T1_ADOBE_STANDARD_FONTS id;
  };

  t1_font_map const t1_names[T1_NUM_FACES] = {
      { "Courier-Bold",          T1_COURIER_BOLD },
      { "Courier-BoldOblique",   T1_COURIER_BOLDOBLIQUE },
      { "Courier-Oblique",       T1_COURIER_OBLIQUE },
      { "Courier",               T1_COURIER },
      { "Helvetica-Bold",        T1_HELVETICA_BOLD },
      { "Helvetica-BoldOblique", T1_HELVETICA_BOLDOBLIQUE },
      { "Helvetica-Oblique",     T1_HELVETICA_OBLIQUE },
      { "Helvetica",             T1_HELVETICA },
      { "Symbol",                T1_SYMBOL },
      { "Times-Bold",            T1_TIMES_BOLD },
      { "Times-BoldItalic",      T1_TIMES_BOLDITALIC },
      { "Times-Italic",          T1_TIMES_ITALIC },
      { "Times-Roman",           T1_TIMES_ROMAN },
      { "ZapfDingbats",          T1_ZAPFDINGBATS }
  };

  struct cmp_glyph_unicode_p
  {
      int operator()(t1s_glyph const& lhs, t1s_glyph const& rhs)
      {
          return (rhs.unicode -lhs.unicode) > 0;
      }
  };

  struct cmp_glyph_charcode_p
  {
      int operator()(t1s_glyph const& lhs, t1s_glyph const& rhs)
      {
          return (rhs.code -lhs.code) > 0;
      }
  };

  // Search the face glyph table for given code. Based on given parameters it
  // can be a lookup keyed by a codepoint or a built-in code.
  template<class Pred, jag::Int (t1s_glyph::*search_key)>
  Int find_width(Int code, t1s_face const* face)
  {
      t1s_glyph const*const end = face->glyphs + face->num_glyphs;

      t1s_glyph glyph;
      (glyph.*search_key) = code;

      t1s_glyph const* found =
          std::lower_bound(face->glyphs, end, glyph, Pred());

      if (found==end || (found->*search_key) != code)
          return face->FontMetrics.missing_width;

      return found->widthx;
  }
}


T1AdobeStandardFace::T1AdobeStandardFace(FontSpecImpl const& spec)
{
    t1_font_map const*const end = &t1_names[0] + T1_NUM_FACES;
    Char const* facename = spec.facename();

    for(t1_font_map const* it = &t1_names[0]; it != end; ++it)
    {
        if (!strcmp(it->name, facename))
        {
            m_face = g_adobe_standard_t1_faces[it->id];
            return;
        }
    }

    throw exception_operation_failed(msg_cannot_find_font(facename)) << JAGLOC;
}


// Searches for width of a character with code 'code'. Code can be either a
// Unicode codepoint or a built-in code.
Int T1AdobeStandardFace::char_horizontal_advance(Int code) const
{
    return !m_face->IsBuiltinEncoding
        ? find_width<cmp_glyph_unicode_p, &t1s_glyph::unicode>(code, m_face)
        : find_width<cmp_glyph_charcode_p, &t1s_glyph::code>(code, m_face);
}


Int T1AdobeStandardFace::gid_horizontal_advance(UInt /*gid*/) const
{
    throw exception_invalid_operation() << JAGLOC;
}


UInt16 T1AdobeStandardFace::codepoint_to_gid(Int /*codepoint*/) const
{
    throw exception_invalid_operation() << JAGLOC;
}

FaceCharIterator T1AdobeStandardFace::char_iterator() const
{
    throw exception_invalid_operation() << JAGLOC;
}

Panose const& T1AdobeStandardFace::panose() const
{
    throw exception_invalid_operation() << JAGLOC;
}

//
//
//
Int T1AdobeStandardFace::kerning_for_gids(UInt /*left*/, UInt /*right*/) const
{
    JAG_INTERNAL_ERROR;
}

Int T1AdobeStandardFace::kerning_for_chars(Int left, Int right) const
{
    return t1_get_kerning(*m_face, left, right);
}


std::auto_ptr<IStreamInput>
T1AdobeStandardFace::font_program(int /*index*/, unsigned /*options*/) const
{
    JAG_PRECONDITION(!"should not be invoked");
    return std::auto_ptr<IStreamInput>();
}

std::auto_ptr<IStreamInput>
T1AdobeStandardFace::subset_font_program(UsedGlyphs const& /*glyphs*/,
                                         unsigned /*options*/) const
{
    JAG_INTERNAL_ERROR;
}




}} // namespace jag::resources

/** EOF @file */
