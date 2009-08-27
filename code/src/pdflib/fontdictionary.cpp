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
    void get_used_codepoints(std::set<Int>& /*codepoints*/, UnicodeConverter* /*conv*/) const
    {
        JAG_INTERNAL_ERROR;
    }

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
    void get_used_codepoints(std::set<Int>& codepoints, UnicodeConverter* conv) const
    {
        JAG_PRECONDITION(conv);
        Char c;
        for(int i=0; i<256; ++i)
        {
            if (m_chars.test(i))
            {
                Char const* p = &c;
                c = static_cast<Char>(static_cast<unsigned char>(i));
                codepoints.insert(conv->next_code_point(&p, p+1));
                JAG_ASSERT(p == &c+1);
            }
        }
    };

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
    UsedCodepoints(ITypeface const& face)
        : m_face(face)
    {}

    void use_codepoints(Int const* start, Int const* end, std::vector<UInt16>& gids) {
        // for each codepoint find its corresponding gid, use m_cp_to_gid as
        // a cache; when codepoint is not present in the font then a warning
        // message is issued (on the first occurence of such codepoint
        for(; start!=end; ++start)
        {
            CodePointToGidMap::iterator it = m_cp_to_gid.find(*start);
            if (it == m_cp_to_gid.end())
            {
                Int gid = m_face.codepoint_to_gid(*start);

                if (!gid)
                {
                    write_message(WRN_CODEPOINT_NOT_FOUND_IN_FONT_uw
                                   , *start
                                   , m_face.family_name());
                }

                it = m_cp_to_gid.insert(std::make_pair(*start, gid)).first;
            }
            gids.push_back(it->second);
        }
    }

    void get_used_codepoints(std::set<Int>& codepoints, UnicodeConverter*) const
    {
        CodePointToGidMap::const_iterator end=m_cp_to_gid.end();
        for(CodePointToGidMap::const_iterator it=m_cp_to_gid.begin(); it!=end; ++it)
            codepoints.insert(it->first);
    };


    void get_used_cids(std::vector<UInt16>& cids) const
    {
        cids.resize(m_cp_to_gid.size());
        std::transform(m_cp_to_gid.begin(), m_cp_to_gid.end(), cids.begin(), jag_select2nd<CodePointToGidMap::value_type>());
        std::sort(cids.begin(), cids.end());
    }


    void output_dictionary(FontDictionary& dict)
    {
        dict.object_writer()
            .dict_start()
            .dict_key("Type").output("Font")
            .dict_key("Subtype").output("Type0")
            .dict_key("BaseFont").name(dict.m_font_descriptor->basename())
            .dict_key("Encoding").output("Identity-H")
            .dict_key("DescendantFonts").array_start().ref(m_cid_font).array_end()
            .dict_key("ToUnicode").space().ref(m_to_unicode)
            .dict_end()
    ;
    }


    bool before_output_dictionary(FontDictionary& dict)
    {
        if (m_cp_to_gid.empty())
        {
            throw exception_operation_failed(
                msg_no_chars_used_from_font()) << JAGLOC;
        }

        JAG_ASSERT(m_cp_to_gid.size());

        CIDFontDictionary cid_font_dict(dict.doc(), dict);
        cid_font_dict.output_definition();
        m_cid_font = IndirectObjectRef(cid_font_dict);

        ToUnicode to_unicode;
        std::vector<ToUnicode::GidAndUnicode> gids(m_cp_to_gid.size());
        ToUnicode::GidAndUnicode* pgid = &gids[0];
        CodePointToGidMap::iterator end = m_cp_to_gid.end();
        for (CodePointToGidMap::iterator it=m_cp_to_gid.begin(); it!=end; ++it, ++pgid)
        {
            pgid->gid = it->second;
            pgid->codepoint[0] = it->first;
        }
        std::sort(gids.begin(), gids.end());
        to_unicode.output_definition(dict.doc(), &gids[0], gids.size());
        m_to_unicode = to_unicode.ref();
        return true;
    }


private:
    typedef std::map<Int,Int>   CodePointToGidMap;
    CodePointToGidMap               m_cp_to_gid;
    ITypeface const&                m_face;
    IndirectObjectRef               m_to_unicode;
    IndirectObjectRef               m_cid_font;
};




//////////////////////////////////////////////////////////////////////////
class FontDictionary::UsedCids
    : public IUsedCharsHandler
{
public:
    void use_cids(Char const* start, Char const* end)
    {
        JAG_PRECONDITION(start!=end);
        JAG_PRECONDITION(!((end-start)%2));

        while(start!=end)
        {
            UInt16 val = static_cast<unsigned char>(*start++);
            m_cids.insert(val | static_cast<unsigned char>(*start++));
        }
    }

    void get_used_codepoints(std::set<Int>& codepoints, UnicodeConverter* conv) const
    {
        JAG_PRECONDITION(conv);
        Char buffer[2];
        Cids::const_iterator end(m_cids.end());
        for(Cids::const_iterator it(m_cids.begin()); it!=end; ++it)
        {
            buffer[0] = static_cast<unsigned char>(*it);
            buffer[1] = static_cast<unsigned char>(*it>>8);
            Char const* curr=buffer;
            codepoints.insert(conv->next_code_point(&curr, buffer+2));
            JAG_ASSERT(curr==buffer+2);
        }
    };


    void get_used_cids(std::vector<UInt16>& cids) const
    {
        cids.resize(m_cids.size());
        std::copy(m_cids.begin(), m_cids.end(), cids.begin());
    }


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
            m_used_chars_handler.reset(new UsedCids);
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


void FontDictionary::get_used_cids(std::vector<UInt16>& cids) const
{
    m_used_chars_handler->get_used_cids(cids);
}



//////////////////////////////////////////////////////////////////////////
void FontDictionary::get_used_codepoints(std::set<Int>& codepoints) const
{
    UnicodeConverter* conv(acquire_converter());
    ON_BLOCK_EXIT_OBJ(*this, & FontDictionary::release_converter);
    m_used_chars_handler->get_used_codepoints(codepoints, conv);
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
void FontDictionary::use_cids(Char const* start, Char const* end)
{
    JAG_PRECONDITION(PDFFontData::COMPOSITE_FONT == m_font_data.font_type());
    JAG_PRECONDITION(ENC_IDENTITY != m_font_data.font_encoding());
    m_used_chars_handler->use_cids(start, end);
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
