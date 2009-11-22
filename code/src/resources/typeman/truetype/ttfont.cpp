// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <resources/typeman/truetype/ttfont.h>
#include <resources/typeman/truetype/ttfontmaker.h>
#include <resources/typeman/truetype/ttstructs.h>
#include <resources/interfaces/typeface.h>
#include <core/generic/null_deleter.h>
#include <core/generic/assert.h>
#include <core/jstd/tracer.h>
#include <map>

using namespace boost::integer;

namespace jag {
namespace resources {
namespace truetype {

namespace
{
  // composite glyph flags
  enum {
      ARG_1_AND_2_ARE_WORDS        = 1u << 0,
      WE_HAVE_A_SCALE            = 1u << 3,
      MORE_COMPONENTS            = 1u << 5,
      WE_HAVE_AN_X_AND_Y_SCALE    = 1u << 6,
      WE_HAVE_A_TWO_BY_TWO        = 1u << 7
  };
}

//////////////////////////////////////////////////////////////////////////
TTFont::TTFont(IStreamInput& font_data)
    : m_ttparser(font_data)
    , m_font(&font_data, &null_deleter)
{
}


//////////////////////////////////////////////////////////////////////////
void TTFont::make_subset(ISeqStreamOutput& subset_font,
                         UsedGlyphs const& used_glyphs,
                         bool include_cmap)
{
    TTFontMaker font_maker;
    font_maker.set_codepoint_to_glyph(used_glyphs.codepoint_to_glyph());


    // Iterate over the used glyphs and insert them to fontmaker. Construct
    // additonal_glyphs for those referenced from composite glyphs.
    typedef std::set<UInt16> Glyphs;
    Glyphs additional_glyphs;
    typedef UsedGlyphs::Glyphs::const_iterator GlyphIterator;
    GlyphIterator end = used_glyphs.glyphs_end();
    for(GlyphIterator it = used_glyphs.glyphs_begin(); it!=end; ++it)
    {
        // load the glyph and add it to font maker
        m_ttparser.load_glyph(*it);
        font_maker.add_glyph(m_ttparser.current_glyph_data(),
                             m_ttparser.current_glyph_size(),
                             *it);

        // inspect the glyph
        if (m_ttparser.current_glyph_size())
        {
            tt_glyph_data const* glyph_data =
                static_cast<tt_glyph_data const*>(m_ttparser.current_glyph_data());

            // is it a composite glyph?
            if (static_cast<short>(glyph_data->m_number_of_contours) < 0)
            {
                Byte const* curr =
                    static_cast<Byte const*>(m_ttparser.current_glyph_data()) + sizeof(tt_glyph_data);
                
                unsigned short flags;
                do {
                    flags = static_cast<unsigned short>(*reinterpret_cast<ubig16_t const*>(curr));
                    ubig16_t const* c_glyph_index = reinterpret_cast<ubig16_t const*>(curr+2);
                    curr += 4;
                    
                    // verify that the glyph is not already in the passed set,
                    if (!used_glyphs.glyphs().count(*c_glyph_index))
                        additional_glyphs.insert(*c_glyph_index);
                    
                    curr += flags & ARG_1_AND_2_ARE_WORDS ? 4 : 2;
                    if (flags & WE_HAVE_A_SCALE)
                        curr += 2;
                    
                    if (flags & WE_HAVE_AN_X_AND_Y_SCALE)
                        curr += 4;
                    
                    if (flags & WE_HAVE_A_TWO_BY_TWO)
                        curr += 8;
                }
                while(flags & MORE_COMPONENTS);
            }
        }
    }
    
    if (!additional_glyphs.empty())
    {
        // upload the additional glyphs to fontmaker
        Glyphs::iterator endg = additional_glyphs.end();
        for(Glyphs::iterator it = additional_glyphs.begin(); it!=endg; ++it)
        {
            m_ttparser.load_glyph(*it);
            font_maker.add_glyph(m_ttparser.current_glyph_data(),
                                 m_ttparser.current_glyph_size(),
                                 *it);
        }
    }

    // The subset can contain no outlines, i.e it for instance could have only
    // spaces with varying widths. In such case the .notdef glyph (index 0) is
    // added to the subset. Otherwise, a missing glyf table causes problems for
    // e.g. Reader or certain FreeType versions.
    if (!font_maker.has_outlines())
    {
        // search through the first 255 glyph slots for one with glyph outlines
        UInt16 i = 0;
        const UInt16 NGLYPHS = 255;
        for(; i < NGLYPHS; ++i)
        {
            m_ttparser.load_glyph(i);
            if (m_ttparser.current_glyph_size())
            {
                font_maker.add_glyph(m_ttparser.current_glyph_data(),
                                     m_ttparser.current_glyph_size(),
                                     i);
                break;
            }
        }

        if (i >= NGLYPHS) {
            TRACE_WRN << "font subset has an empty glyph table";
        }
    }
    

    TTFontParser::TableData table_data(m_ttparser.load_table(TT_MAXP));
    font_maker.add_table(TT_MAXP, table_data.first, table_data.second);

    table_data = m_ttparser.load_table(TT_HEAD);
    font_maker.add_table(TT_HEAD, table_data.first, table_data.second);


    // spec (5.8) says that name, os2 and post should not be needed
    // but when really removed Acrobat does not behave well
    const int num_const_tables = 8;
    const TTTableType const_tables[num_const_tables] = {
        TT_NAME, TT_OS2, TT_CVT, TT_FPGM, TT_PREP, TT_HHEA, TT_HMTX, TT_POST
    };

    for (int i=0; i<num_const_tables; ++i)
    {
        TTFontParser::TableData tdata(m_ttparser.load_table(const_tables[i]));
        if (tdata.first)
            font_maker.add_table(const_tables[i], tdata.first, tdata.second);
    }

    // to FontMaker::add_glyph() signature
    font_maker.output(subset_font, include_cmap);
}

//////////////////////////////////////////////////////////////////////////
std::pair<void const*,size_t> TTFont::dbg_load_glyph(UInt codepoint)
{
    unsigned gid = m_ttparser.charcode_to_glyph_index(codepoint);
    m_ttparser.load_glyph(gid);
    return std::make_pair(
          m_ttparser.current_glyph_data()
        , m_ttparser.current_glyph_size()
   );
}

//////////////////////////////////////////////////////////////////////////
Char const* TTFont::postscript_name()
{
    return m_ttparser.postscript_name();
}


}}} // namespace jag::resources::truetype

