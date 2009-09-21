// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "cidfontdictionary.h"
#include "pdffont.h"
#include "objfmt.h"
#include "docwriterimpl.h"
#include "resourcemanagement.h"
#include "fontdictionary.h"
#include "fontdescriptor.h"
#include "datatypecasts.h"
#include <core/jstd/unicode.h>
#include <core/generic/scopeguard.h>
#include <resources/typeman/typefaceutils.h>


using namespace jag::jstd;

namespace jag {
namespace pdf {

namespace
{

}


//////////////////////////////////////////////////////////////////////////
CIDFontDictionary::CIDFontDictionary(DocWriterImpl& doc, FontDictionary& dict)
    : IndirectObjectImpl(doc)
    , m_fdict(dict)
{
    JAG_PRECONDITION(PDFFontData::COMPOSITE_FONT == m_fdict.fdict_data().font_data().font_type());
}


//////////////////////////////////////////////////////////////////////////
void CIDFontDictionary::on_output_definition()
{
    // !! As far CIDSystemInfo dictionary is concerned at the moment of writing this I'm not
    // quite sure whether it has any sense to use any TrueType fonts with predefined
    // CIDSystemInfo dictionaries or Identity should be always used.
    //
    // If it is not really possible then we have to detect it sooner during font registration
    // and set font encoding to identity when TrueType is used

    std::pair<char const*, int> collection = character_collection_str(m_fdict.fdict_data().font_encoding(), doc().version());

    object_writer()
        .dict_start()
        .dict_key("Type").output("Font")
        .dict_key("BaseFont").name(m_fdict.font_descriptor()->basename())
        .dict_key("FontDescriptor").space().ref(*m_fdict.font_descriptor())
        .dict_key("CIDSystemInfo")
        .dict_start()
        .dict_key("Registry").text_string("Adobe")
        .dict_key("Ordering").text_string(collection.first)
        .dict_key("Supplement").space().output(collection.second)
        .dict_end()
    ;



    // typeface distingiuish type2 and type0 based on typeface type
    object_writer().dict_key("Subtype");
    FaceType face_type = m_fdict.fdict_data().font_data().typeface().type();
    switch(face_type)
    {
    case FACE_TRUE_TYPE:
        object_writer().output("CIDFontType2");
        break;

    case FACE_OPEN_TYPE_CFF:
        object_writer().output("CIDFontType0");
        break;

    default:
        // find out what is allowed here
        JAG_TBD;
    }


    output_widths();
    object_writer().dict_end();
}


//////////////////////////////////////////////////////////////////////////
void CIDFontDictionary::output_widths()
{
    ITypeface const& face(m_fdict.fdict_data().typeface());
    resources::FontSpaceToGlyphSpace fs2gs(face.metrics());

    ObjFmt& writer(object_writer());

    writer
        .dict_key("DW")
        .space()
        .output(fs2gs(face.metrics().missing_width))
    ;


    // retrieve used cids
    UsedGlyphs::Glyphs const& used_glyphs(m_fdict.get_used_cids().glyphs());
    std::vector<UInt16> cids;
    cids.reserve(used_glyphs.size());
    std::copy(used_glyphs.begin(), used_glyphs.end(), std::back_inserter(cids));

    std::vector<Int> widths;

    JAG_ASSERT(cids.size());
    fetch_widths(cids, widths);

    writer
        .dict_key("W")
        .array_start()
    ;

    UInt start_cid = cids[0];
    size_t start_cid_index = 0;
    enum { RANGE_CLOSED, RANGE_OPENED } status = RANGE_CLOSED;
    const size_t cids_len = cids.size();
    for(size_t i=1; i<cids_len; ++i)
    {
        if (RANGE_CLOSED == status)
        {
            // range start
            writer
                .output(start_cid)
                .array_start()
                .output(widths[start_cid_index])
            ;
            status = RANGE_OPENED;
        }

        if (cids[i] == cids[i-1]+1)
        {
            // range continuation
            JAG_ASSERT(status == RANGE_OPENED);
            writer
                .space()
                .output(widths[i])
            ;
        }
        else
        {
            // range end
            writer.array_end();
            status = RANGE_CLOSED;
            start_cid = cids[i];
            start_cid_index = i;
        }
    }

    if (RANGE_OPENED == status)
    {
        writer.array_end();
    }
    else
    {
        // last orphaned gid
        writer
            .output(start_cid)
            .array_start()
            .output(widths[start_cid_index] )
            .array_end()
        ;
    }

    writer.array_end();
}



/**
 * @brief For given vector of cids fetches corresponding widths from typeface.
 *
 * @param cids encoded characters or glyph indices (determined by font dictionary encoding)
 * @param widths filled in with corresponding widhts (in glyph space)
 */
void CIDFontDictionary::fetch_widths(std::vector<UInt16> const& cids, std::vector<Int>& widths)
{
    ITypeface const& face(m_fdict.fdict_data().typeface());
    resources::FontSpaceToGlyphSpace fs2gs(face.metrics());

    // Font dictionary can be formed either by encoded characters (cids) or
    // glyph indices (gids).
    widths.reserve(cids.size());
    const size_t cids_len = cids.size();
    if (ENC_IDENTITY == m_fdict.fdict_data().font_encoding())
    {
        // gids
        for(size_t i=0; i<cids_len; ++i)
            widths.push_back(fs2gs(face.gid_horizontal_advance(cids[i])));
    }
    else
    {
        // cids
        std::vector<Char> cids_byte_arr(cids_len*2);
        for(size_t i=0,j=0; i<cids_len; ++i)
        {
            cids_byte_arr[++j] = static_cast<Char>(cids[i] & 8);
            cids_byte_arr[++j] = static_cast<Char>(cids[i] >> 8);
        }

        UnicodeConverter* conv(m_fdict.acquire_converter());
        JAG_ASSERT(conv);
        ON_BLOCK_EXIT_OBJ(m_fdict, &FontDictionary::release_converter);
        std::vector<Int> codepoints;
        codepoints.reserve(cids_len);

        JAG_ASSERT(cids_byte_arr.size());
        Char const* cids_start = &cids_byte_arr[0];
        Char const*const cids_end = cids_start + cids_byte_arr.size();
        while (cids_start!=cids_end)
            codepoints.push_back(conv->next_code_point(&cids_start, cids_end));

        JAG_ASSERT_MSG(cids_len==codepoints.size(), "?number of codepoints should correspond to #cids");
        for(size_t i=0; i<cids_len; ++i)
            widths.push_back(fs2gs(face.char_horizontal_advance(codepoints[i])));
    }
}


}} //namespace jag::pdf

