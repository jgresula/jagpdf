// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "fontdictionary.h"
#include "objfmt.h"
#include "fontdescriptor.h"
#include "tounicode.h"
#include "cidfontdictionary.h"
#include "datatypecasts.h"
#include "docwriterimpl.h"
#include "resourcemanagement.h"
#include "encdifferences.h"
#include <msg_pdflib.h>
#include <resources/interfaces/typeface.h>
#include <resources/typeman/typefaceutils.h>
#include <core/generic/scopeguard.h>
#include <core/jstd/unicode.h>
#include <core/errlib/msg_writer.h>
#include <core/errlib/errlib.h>
#include <core/generic/assert.h>
#include <core/generic/functional.h>
#include <map>
#include <set>
#include <bitset>


using namespace jag::jstd;
using namespace boost;

namespace jag {
namespace pdf {


///////////////////////////////////////////////////////////////////////////
class FontDictionary::SimpleFontHandlerBase
    : public IUsedCharsHandler
{
 protected:
    std::bitset<256> m_chars;
    typedef std::pair<int,int> CharsRange;

    CharsRange used_chars_range() const
    {
        if (m_chars.none())
        {
            throw exception_operation_failed(
                msg_no_chars_used_from_font()) << JAGLOC;
        }


        int first=0;
        while(first<256 && !m_chars.test(first)) ++first;
        JAG_ASSERT(first<256);

        int last = 255;
        while(last>=0 && !m_chars.test(last)) --last;
        JAG_ASSERT(last>=0 && last >= first);

        return CharsRange(first, last);
    }

    void output_widths(FontDictionary& dict, CharsRange rng)
    {
        const size_t num_char_codes = rng.second-rng.first+1;
        JAG_PRECONDITION(num_char_codes < 257);
        JAG_PRECONDITION(rng.second < 256);

        ObjFmt& writer = dict.object_writer();
        writer
            .dict_key("Widths")
            .array_start()
            ;

        UnicodeConverter* conv(dict.acquire_converter());
        JAG_ASSERT(conv);
        ON_BLOCK_EXIT_OBJ(dict, &FontDictionary::release_converter);

        ITypeface const& face(dict.fdict_data().typeface());
        resources::FontSpaceToGlyphSpace fs2gs(face.metrics());

        char buffer[256];
        char* it_set = buffer;
        char const*const end = buffer+num_char_codes;
        while (it_set!=end)
            *it_set++ = static_cast<char>(rng.first++);

        for(char const* it = buffer; it!=end;)
        {
            Int cp = conv->next_code_point(&it, end);
            writer.output(fs2gs(face.char_horizontal_advance(cp))).space();
        }

        writer.array_end();
    }

public:
    void use_char8(Char const* start, Char const* end)
    {
        JAG_PRECONDITION(start!=end);
        for(; start!=end; ++start)
            m_chars.set(static_cast<unsigned char>(*start));
    }
};


//////////////////////////////////////////////////////////////////////////
class FontDictionary::Standard14T1Handler
    : public SimpleFontHandlerBase
{
    IndirectObjectRef m_enc_dict;

public:
    bool before_output_dictionary(FontDictionary& dict)
    {
        if (supports_differences(dict.m_font_data.font_encoding()))
        {
            m_enc_dict = dict.doc().res_mgm().fonts().enc_diff_dict(dict.m_font_data.font_encoding());
        }
        return true;
    }

    void output_dictionary(FontDictionary& dict)
    {
        ITypeface const& face(dict.fdict_data().typeface());

        CharsRange chars_range(used_chars_range());
        ObjFmt& writer(dict.object_writer());
        writer
            .dict_start()
            .dict_key("Type").output("Font")
            .dict_key("Subtype").output("Type1")
            .dict_key("BaseFont").name(face.postscript_name())
        ;

        if (is_valid(m_enc_dict))
        {
            JAG_ASSERT(supports_differences(dict.m_font_data.font_encoding()));

            // we must output widths in case encoding differences are
            // used since adobe reader (8.0) does not handle widths
            // correctly; other viewers (Foxit, GSView) seems to work
            writer
                .dict_key("Encoding").space().ref(m_enc_dict)
                .dict_key("FirstChar").space().output(chars_range.first)
                .dict_key("LastChar").space().output(chars_range.second);
            output_widths(dict, chars_range);
        }
        else
        {
            switch(dict.m_font_data.font_encoding())
            {
            case ENC_PDF_STANDARD:
            case ENC_BUILTIN:
                // no action required
                break;

            case ENC_CP_1252: {
                writer.dict_key("Encoding");
                char const* enc_str = encoding_type_str(dict.m_font_data.font_encoding());
                JAG_ASSERT(enc_str);
                writer.output(enc_str);
                break;
            }

            default:
                JAG_INTERNAL_ERROR;
            }
        }

        writer.dict_end();
    }
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
class FontDictionary::UsedChars8
    : public SimpleFontHandlerBase
{
public:
    void output_dictionary(FontDictionary& dict)
    {
    // it is recommended to use true-type simple font only with the predefined
    // encodings, we extend this rule to all font types (i.e. no encoding differences)
    JAG_ASSERT(
            ENC_CP_1252  == dict.m_font_data.font_encoding()
            || ENC_PDF_STANDARD == dict.m_font_data.font_encoding()
       );
        JAG_ASSERT(dict.m_font_descriptor);

        ITypeface const& face(dict.fdict_data().typeface());
        CharsRange chars_range(used_chars_range());

        dict.object_writer()
            .dict_start()
            .dict_key("Type").output("Font")
            .dict_key("Subtype").output(font_type_str(face))
            .dict_key("BaseFont").name(dict.m_font_descriptor->basename())
            .dict_key("FontDescriptor").space().ref(*dict.m_font_descriptor)
            .dict_key("FirstChar").space().output(chars_range.first)
            .dict_key("LastChar").space().output(chars_range.second)
        ;

        dict.object_writer().dict_key("Encoding");
        char const* enc_str = encoding_type_str(dict.m_font_data.font_encoding());
        JAG_ASSERT(enc_str);
        dict.object_writer().output(enc_str);

        output_widths(dict, chars_range);
        dict.object_writer().dict_end();

    // Missing width shouldn't be needed as widths of all used character codes
    // are listed in the Width array
    }
};




//////////////////////////////////////////////////////////////////////////
class FontDictionary::UsedCodepoints
    : public IUsedCharsHandler
{
public:
    UsedCodepoints(ITypeface const& face);

    void use_codepoints(Int const* start, Int const* end, std::vector<UInt16>& gids);
    void use_gids(UInt16 const* start, UInt16 const* end);
    UsedGlyphs const& get_used_cids() const;
    void output_dictionary(FontDictionary& dict);
    bool before_output_dictionary(FontDictionary& dict);

private:
    UsedGlyphs m_glyphs;
    
    ITypeface const&                m_face;
    IndirectObjectRef               m_to_unicode;
    IndirectObjectRef               m_cid_font;
};




FontDictionary::UsedCodepoints::UsedCodepoints(ITypeface const& face)
    : m_glyphs(face)
    , m_face(face)
{}


//
//
// 
void FontDictionary::UsedCodepoints::use_codepoints(
    Int const* start,
    Int const* end,
    std::vector<UInt16>& gids)
{
    JAG_PRECONDITION(start < end);
    
    // for each codepoint find its corresponding gid
    gids.reserve(end-start);
    m_glyphs.set_update_glyphs();
    for(; start!=end; ++start)
        gids.push_back(m_glyphs.add_codepoint(*start));
}


//
//
// 
void FontDictionary::UsedCodepoints::use_gids(UInt16 const* start, UInt16 const* end)
{
    for(; start != end; ++start)
        m_glyphs.add_glyph(*start);
}

//
//
// 
UsedGlyphs const &
FontDictionary::UsedCodepoints::get_used_cids() const
{
    return m_glyphs;
}


//
//
// 
void FontDictionary::UsedCodepoints::output_dictionary(FontDictionary& dict)
{
    ObjFmt& writer = dict.object_writer();

    writer
        .dict_start()
        .dict_key("Type").output("Font")
        .dict_key("Subtype").output("Type0")
        .dict_key("BaseFont").name(dict.m_font_descriptor->basename())
        .dict_key("Encoding").output("Identity-H")
        .dict_key("DescendantFonts").array_start().ref(m_cid_font).array_end()
        ;

    // ToUnicode object could end up empty
    if (is_valid(m_to_unicode))
        writer.dict_key("ToUnicode").space().ref(m_to_unicode);

    writer.dict_end();
}


//
//
// 
bool FontDictionary::UsedCodepoints::before_output_dictionary(FontDictionary& dict)
{
    m_glyphs.update();
    
    if (m_glyphs.glyphs().empty())
    {
        throw exception_operation_failed(
            msg_no_chars_used_from_font()) << JAGLOC;
    }
 
    CIDFontDictionary cid_font_dict(dict.doc(), dict);
    cid_font_dict.output_definition();
    m_cid_font = IndirectObjectRef(cid_font_dict);

    // if only glyphs with no associated codepoints are present then quit
    if (m_glyphs.codepoint_to_glyph().empty())
        return true;

    // generate a ToUnicode object
    typedef UsedGlyphs::CodepointToGlyph CodepointToGlyph;
    CodepointToGlyph const& codepoint_to_glyph(m_glyphs.codepoint_to_glyph());
    
    ToUnicode to_unicode;
    std::vector<ToUnicode::GidAndUnicode> gids(codepoint_to_glyph.size());
    ToUnicode::GidAndUnicode* pgid = &gids[0];

    typedef CodepointToGlyph::const_iterator Iter;
    Iter end = codepoint_to_glyph.end();
    for (Iter it = codepoint_to_glyph.begin(); it!=end; ++it, ++pgid)
    {
        pgid->gid = it->second;
        pgid->codepoint[0] = it->first;
    }
    std::sort(gids.begin(), gids.end());
    to_unicode.output_definition(dict.doc(), &gids[0], gids.size());
    m_to_unicode = to_unicode.ref();
    return true;
}




//////////////////////////////////////////////////////////////////////////
class FontDictionary::UsedCids
    : public IUsedCharsHandler
{
public:
    void output_dictionary(FontDictionary& dict)
    {
        dict.object_writer()
            .dict_start()
            .dict_key("Type").output("Font")
            .dict_key("Subtype").output("Type0")
            .dict_key("BaseFont").name(dict.m_font_descriptor->basename())
            .dict_key("Encoding").output(cmap_str(dict.m_font_data.font_encoding()))
            .dict_key("DescendantFonts").array_start().ref(m_cid_font).array_end()
            .dict_end()
    ;
    }


    bool before_output_dictionary(FontDictionary& dict)
    {
        JAG_ASSERT(m_cids.size());

        CIDFontDictionary cid_font_dict(dict.doc(), dict);
        cid_font_dict.output_definition();
        m_cid_font = IndirectObjectRef(cid_font_dict);
        return true;
    }

private:
    typedef std::set<UInt16> Cids;
    Cids m_cids;
    IndirectObjectRef m_cid_font;
};






//////////////////////////////////////////////////////////////////////////
// class FontDictionary
//////////////////////////////////////////////////////////////////////////
FontDictionary::FontDictionary(DocWriterImpl& doc,
                                int id,
                                PDFFontDictData const& data,
                                Char const* encoding)
    : IndirectObjectImpl(doc)
    , m_font_data(data)
    , m_conv_ctrl(ENC_IDENTITY != data.font_encoding() ? encoding : "")
    , m_id(id)
{
    if (PDFFontData::SIMPLE_FONT == data.font_type())
    {
        if (FACE_TYPE_1_ADOBE_STANDARD == data.typeface().type())
        {
            m_used_chars_handler.reset(new Standard14T1Handler);
        }
        else
        {
            m_used_chars_handler.reset(new UsedChars8);
        }
    }
    else
    {
        if (ENC_IDENTITY == data.font_encoding())
        {
            m_used_chars_handler.reset(new UsedCodepoints(data.typeface()));
        }
        else
        {
            // this path is not implemented
            JAG_INTERNAL_ERROR;
            //m_used_chars_handler.reset(new UsedCids);
        }
    }

    JAG_POSTCONDITION(m_used_chars_handler);
}



//////////////////////////////////////////////////////////////////////////
UnicodeConverter* FontDictionary::acquire_converter() const
{
    return m_conv_ctrl.acquire_converter();
}



//////////////////////////////////////////////////////////////////////////
void FontDictionary::release_converter() const
{
    m_conv_ctrl.release_converter();
}



//////////////////////////////////////////////////////////////////////////
void FontDictionary::on_output_definition()
{
    try
    {
        m_used_chars_handler->output_dictionary(*this);
    }
    catch(exception const& exc)
    {
        throw exception_operation_failed(
            msg_cannot_output_font(
                fdict_data().typeface().full_name().c_str()), &exc) << JAGLOC;
    }
}



bool FontDictionary::on_before_output_definition()
{
    try
    {
        return m_used_chars_handler->before_output_dictionary(*this);
    }
    catch(exception const& exc)
    {
        throw exception_operation_failed(
            msg_cannot_output_font(
                fdict_data().typeface().full_name().c_str()), &exc) << JAGLOC;
    }
}


//
//
// 
UsedGlyphs const &FontDictionary::get_used_cids() const
{
    return m_used_chars_handler->get_used_cids();
}


//////////////////////////////////////////////////////////////////////////
void FontDictionary::set_font_descriptor(boost::shared_ptr<FontDescriptor> const& desc)
{
    m_font_descriptor = desc;
}



//////////////////////////////////////////////////////////////////////////
void FontDictionary::use_char8(Char const* start, Char const* end)
{
    JAG_PRECONDITION(PDFFontData::SIMPLE_FONT == m_font_data.font_type());
    m_used_chars_handler->use_char8(start, end);
}


//////////////////////////////////////////////////////////////////////////
void FontDictionary::use_codepoints(Int const* start, Int const* end, std::vector<UInt16>& gids)
{
    JAG_PRECONDITION(PDFFontData::COMPOSITE_FONT == m_font_data.font_type());
    JAG_PRECONDITION(ENC_IDENTITY == m_font_data.font_encoding());
    m_used_chars_handler->use_codepoints(start, end, gids);
}



//////////////////////////////////////////////////////////////////////////
void FontDictionary::use_gids(UInt16 const* start, UInt16 const* end)
{
    JAG_PRECONDITION(PDFFontData::COMPOSITE_FONT == m_font_data.font_type());
    JAG_PRECONDITION(ENC_IDENTITY == m_font_data.font_encoding());
    m_used_chars_handler->use_gids(start, end);
}

//////////////////////////////////////////////////////////////////////////
boost::shared_ptr<FontDescriptor> const& FontDictionary::font_descriptor() const
{
    JAG_PRECONDITION(m_font_descriptor);
    return m_font_descriptor;
}


//
// explicit instantiation of the following ObjFmtBasic template is here to avoid
// dependencise on FontDictionary in ObjFmtBasic
//
template ObjFmtBasic& ObjFmtBasic::output_resource(FontDictionary const& handle);


}} //namespace jag::pdf
