// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "freetypeopenargs.h"
#include <core/generic/checked_cast.h>
#include <core/generic/autoarray.h>
#include <core/jstd/memory_stream.h>
#include <core/jstd/crt_platform.h>
#include <core/errlib/errlib.h>
#include <core/jstd/tracer.h>
#include <resources/typeman/typefaceimpl.h>
#include <resources/typeman/typefaceutils.h>
#include <resources/typeman/truetypetable.h>
#include <resources/typeman/truetype/ttfont.h>
#include <msg_resources.h>
#include <interfaces/streams.h>
#include <boost/functional/hash.hpp>

#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_TAGS_H
#include <freetype/ftxf86.h>

using namespace jag::jstd;
using namespace jag::resources::truetype;

namespace jag {
namespace resources {

namespace
{

  /// retrieves double from 16/16 fixed point
  inline double double_from_16_16(FT_Fixed val)
  {
      return val/65536.0;
  }

} // anonymous namespace

//////////////////////////////////////////////////////////////////////////
TypefaceImpl::TypefaceImpl(boost::shared_ptr<FT_LibraryRec_> ftlib,
                           std::auto_ptr<FTOpenArgs> args)
    : m_open_args(args.release())
    , m_face(0)
    , m_type(FACE_UNINITIALIZED)
    , m_can_embed(false)
    , m_can_subset(false)
    , m_ftlib(ftlib)
{
    FT_Error err = FT_Open_Face(
        m_ftlib.get(), m_open_args->get_args(0), 0, &m_face);
    CHECK_FT(err);
    
    if (2 == m_open_args->num_records())
    {
        FT_Error err = FT_Attach_Stream(m_face, m_open_args->get_args(1));
        CHECK_FT(err);
    }

    calculate_hash();
    detect_type();
    preflight();
    
    JAG_POSTCONDITION(m_face);
}


//////////////////////////////////////////////////////////////////////////
void TypefaceImpl::detect_type()
{
    if (FT_IS_SFNT(m_face))
    {
        // detect opentype with truetype outlines: check if there is glyf table,
        // otherwise the font should have postcript outlines
        FT_ULong table_len = 0;
        FT_Load_Sfnt_Table(
            m_face, FT_MAKE_TAG('g', 'l', 'y', 'f'), 0, 0, &table_len);
        
        if (table_len)
        {
            m_type = FACE_TRUE_TYPE;
        }
        else
        {
            FT_Load_Sfnt_Table(
                m_face, FT_MAKE_TAG('C', 'F', 'F', ' '), 0, 0, &table_len);
            
            if (table_len)
            {
                m_type = FACE_OPEN_TYPE_CFF;
            }
            JAG_ASSERT_MSG(table_len, "weird sfnt");
        }

        // select unicode charmap
        if (m_type == FACE_TRUE_TYPE || m_type == FACE_OPEN_TYPE_CFF)
        {
            FT_Error err = FT_Select_Charmap(m_face, FT_ENCODING_UNICODE);
            CHECK_FT(err);
        }
    }

    if (m_type == FACE_UNINITIALIZED)
        throw exception_invalid_input(msg_unknown_font_format()) << JAGLOC;

    JAG_POSTCONDITION(m_type != FACE_UNINITIALIZED);
}



//
//
// 
TypefaceMetrics const& TypefaceImpl::metrics() const
{
    return m_metrics;
}



//////////////////////////////////////////////////////////////////////////
void TypefaceImpl::calculate_hash()
{
    // md5 on len + first 1020 bytes of the first stream
    const int buff_size = 1024;
    int size((std::min)(data_size(0), buff_size));
    Byte buffer[buff_size];
    if (size<buff_size)
        memset(buffer+size, 0, buff_size-size);

    *jag_reinterpret_cast<unsigned int*>(buffer) = data_size(0);
    size -= sizeof(unsigned int);

    ULong read;
    font_program(0, 0)->read(buffer+sizeof(unsigned int), size, &read);
    if (size!=static_cast<int>(read))
        throw std::runtime_error("cannot read typeface");
    m_md5.append(buffer, buff_size);
    m_md5.finish();
}



//////////////////////////////////////////////////////////////////////////
TypefaceImpl::~TypefaceImpl()
{
    FT_Done_Face(m_face);
}


//
//
// 
int TypefaceImpl::num_streams() const
{
    return m_open_args->num_records();
}



//////////////////////////////////////////////////////////////////////////
std::auto_ptr<IStreamInput>
TypefaceImpl::font_program(int index, unsigned options) const
{
    JAG_PRECONDITION(index >=0 && index < num_streams());
    if (options & EXTRACT_CFF)
    {
        JAG_ASSERT(m_type == FACE_OPEN_TYPE_CFF);
        FT_ULong cff_len = 0;
        FT_Error err = FT_Load_Sfnt_Table(
            m_face, FT_MAKE_TAG('C', 'F', 'F', ' '), 0, 0, &cff_len);
        
        CHECK_FT(err);
        auto_array<Byte> cff_table(cff_len);
        err = FT_Load_Sfnt_Table(
            m_face, FT_MAKE_TAG('C', 'F', 'F', ' '),
            0, cff_table.ptr(), &cff_len);
        
        CHECK_FT(err);
        return std::auto_ptr<IStreamInput>(
            new MemoryStreamInput(cff_table.detach(), cff_len, true));
    }

    return create_ftopenargs_stream_adapter(*m_open_args->get_args(index));
}



/// size of the first font stream
int TypefaceImpl::data_size(int index) const
{
    JAG_PRECONDITION(index >=0 && index < num_streams());
    return m_open_args->data_size(index);
}



/// find out basic information about the face + its metrics
void TypefaceImpl::preflight()
{
    // as metrics structure can grow we need to ensure
    // *somehow* that all structure fields are filled in
    // in all cases
    JAG_PRECONDITION(m_type != FACE_UNINITIALIZED);

    memset(&m_metrics, 0, sizeof(TypefaceMetrics));
    // metrics common for all types
    m_metrics.units_per_EM = m_face->units_per_EM;
    m_metrics.bbox_xmin = m_face->bbox.xMin;
    m_metrics.bbox_ymin = m_face->bbox.yMin;
    m_metrics.bbox_xmax = m_face->bbox.xMax;
    m_metrics.bbox_ymax = m_face->bbox.yMax;
    m_metrics.ascent = m_face->ascender;
    m_metrics.descent = m_face->descender;
    m_metrics.max_width = m_face->max_advance_width;
    m_metrics.baseline_distance = m_face->height;
    m_fixed_width = FT_IS_FIXED_WIDTH(m_face);

    // assuming that missing width is stored at gid 0
    CHECK_FT(FT_Load_Glyph(m_face, 0, FT_LOAD_NO_SCALE));
    m_metrics.missing_width = m_face->glyph->metrics.horiAdvance;

    switch (m_type)
    {
    case FACE_TRUE_TYPE:
    case FACE_OPEN_TYPE_CFF: {
        Table<OS2Table> os2(m_face);
        Table<PostscriptTable> ps(m_face);

        // embedding
        // least restrictive license is granted (adobe tech. note #5641)
        if (os2->fsType & 0x8 || os2->fsType & 0x4 || !(os2->fsType & 0x2))
            m_can_embed = true;

        // subsetting
        if (!(os2->fsType & 0x100))
        {
            // cmap format 4 supported only
            for (FT_Int i=0; i< m_face->num_charmaps; ++i)
            {
                FT_CharMap chmap = m_face->charmaps[i];
                if ((chmap->platform_id==3 && chmap->encoding_id==1) ||
                     (chmap->platform_id==0))
                {
                    if (4 == FT_Get_CMap_Format(chmap))
                    {
                        m_can_subset = true;
                        break;
                    }
                }
            }
        }


        m_metrics.avg_width = os2->xAvgCharWidth;

        m_metrics.cap_height = 0;
        m_metrics.xheight = 0;
        if (os2->version >= 2) // open-type?
        {
            m_metrics.cap_height = os2->sCapHeight;
            m_metrics.xheight = os2->sxHeight;
        }
        else
        {
            Table<PCLTTable> pclt(m_face);
            if (pclt.exists())
            {
                m_metrics.cap_height = pclt->CapHeight;
                m_metrics.xheight = pclt->xHeight;
            }
            else
            {
                m_metrics.cap_height = !FT_Load_Char(m_face, 'H', FT_LOAD_NO_SCALE)
                    ? m_face->glyph->metrics.height
                    : m_metrics.ascent
                ;

                if (!FT_Load_Char(m_face, 'x', FT_LOAD_NO_SCALE))
                    m_metrics.xheight = m_face->glyph->metrics.height;
            }
        }

        m_weight_class = os2->usWeightClass;
        m_width_class = os2->usWidthClass;
        m_italic_angle = double_from_16_16(ps->italicAngle);
        memcpy(m_panose, os2->panose, sizeof(m_panose));
        break;
    }
    default:
        ;
    }

    if (m_type == FACE_OPEN_TYPE_CFF)
        m_can_subset = false;   // CFF format subsetting not implemented

    if (m_type == FACE_TYPE_1)
    {
        // Type 1 subsetting and embedding implemented
        m_can_subset = false;
        m_can_embed = false;
    }

    // check that fundamental font fields are present

    // actually not quite sure about this one as font might not define
    // family name in english but in another language
    if (!m_face->family_name)
        throw exception_invalid_input(msg_font_family_not_present()) << JAGLOC;
}



//////////////////////////////////////////////////////////////////////////
Int TypefaceImpl::bold() const
{
    return m_face->style_flags & FT_STYLE_FLAG_BOLD;
}



//////////////////////////////////////////////////////////////////////////
Int TypefaceImpl::italic() const
{
    return m_face->style_flags & FT_STYLE_FLAG_ITALIC;
}



//////////////////////////////////////////////////////////////////////////
Char const* TypefaceImpl::family_name() const
{
    return m_face->family_name;
}



Char const* TypefaceImpl::postscript_name() const
{
    return FT_Get_Postscript_Name(m_face);
}


//////////////////////////////////////////////////////////////////////////
Char const* TypefaceImpl::style_name() const
{
    return m_face->style_name;
}



//////////////////////////////////////////////////////////////////////////
std::string TypefaceImpl::full_name() const
{
    std::string result(family_name());
    Char const* style = style_name();
    if (style && JAG_STRICMP(style, "regular"))
    {
        result += ' ';
        result += style;
    }
    return result;
}







//////////////////////////////////////////////////////////////////////////
std::auto_ptr<IStreamInput>
TypefaceImpl::subset_font_program(UsedGlyphs const& glyphs,
                                  unsigned options) const
{
    JAG_PRECONDITION(m_can_subset);

    if (FACE_TRUE_TYPE == m_type)
    {
        try
        {
            std::auto_ptr<IStreamInput> font_prg(font_program(0,options));
            truetype::TTFont font(*font_prg);

            boost::shared_ptr<MemoryStreamOutput> mem_out(new MemoryStreamOutput);
            bool include_cmap = !(options & DONT_INCLUDE_CMAP);

            font.make_subset(*mem_out, glyphs, include_cmap);
            
            return std::auto_ptr<IStreamInput>(
                new MemoryStreamInputFromOutput(mem_out));
        }
        catch(exception& exc)
        {
            // append the font file path to the exception
            exc << io_object_info(m_open_args->filename(0));
            throw;
        }
    }

    JAG_INTERNAL_ERROR;
}



//////////////////////////////////////////////////////////////////////////
UInt16 TypefaceImpl::codepoint_to_gid(Int codepoint) const
{
    UInt16 gid=0;
    switch(m_type)
    {
    case FACE_TRUE_TYPE:
    case FACE_OPEN_TYPE_CFF:
        gid = static_cast<UInt16>(FT_Get_Char_Index(m_face, codepoint));
        break;

    default:
        JAG_TBD;
    };
    return gid;
}

//
// O(N), where N is number of glyphs in font
// 
FaceCharIterator TypefaceImpl::char_iterator() const
{
    return FaceCharIterator(m_face);
}


Int TypefaceImpl::gid_horizontal_advance(UInt gid) const
{
    if (!FT_Load_Glyph(m_face, gid, FT_LOAD_NO_SCALE))
        return m_face->glyph->metrics.horiAdvance;
    else
        return 0;
}



Int TypefaceImpl::char_horizontal_advance(Int codepoint) const
{
    Int advance=0;
    switch(m_type)
    {
    case FACE_OPEN_TYPE_CFF:
    case FACE_TRUE_TYPE: {
        // if FT_Get_Char_Index returns 0, it means that there is no glyph for
        // that code point, however we can still load glyph at index 0 which
        // should represent missing character by convention
        return gid_horizontal_advance(FT_Get_Char_Index(m_face, codepoint));
    }

    default:
        JAG_TBD;
    }

    return advance;
}

Int TypefaceImpl::kerning_for_gids(UInt left, UInt right) const
{
    FT_Vector delta;
    CHECK_FT(FT_Get_Kerning(m_face, left, right,
                            FT_KERNING_UNSCALED, &delta));
    return delta.x;
}

Int TypefaceImpl::kerning_for_chars(Int left, Int right) const
{
    return kerning_for_gids(codepoint_to_gid(left),
                            codepoint_to_gid(right));
}

} // namespace resources


// ---------------------------------------------------------------------------
//                    class jag::UsedGlyphs

UsedGlyphs::UsedGlyphs(ITypeface const& face)
    : m_face(face)
    , m_impl(new Impl)
{
    m_impl->update_glyphs = false;
    m_impl->update_map = false;
}

bool UsedGlyphs::is_updated() const
{
    return !m_impl->update_map && !m_impl->update_glyphs;
}

UsedGlyphs::CodepointToGlyph const& UsedGlyphs::codepoint_to_glyph() const
{
    JAG_PRECONDITION(is_updated());
    return m_impl->codepoint_to_glyph;
}

UsedGlyphs::Glyphs const& UsedGlyphs::glyphs() const
{
    JAG_PRECONDITION(is_updated());
    return m_impl->glyphs;
}

UsedGlyphs::GlyphsIter UsedGlyphs::glyphs_begin() const
{
    JAG_PRECONDITION(is_updated());
    return m_impl->glyphs.begin();
}

UsedGlyphs::GlyphsIter UsedGlyphs::glyphs_end() const
{
    JAG_PRECONDITION(is_updated());
    return m_impl->glyphs.end();
}

UInt16 UsedGlyphs::add_codepoint(Int codepoint)
{
    JAG_PRECONDITION(m_impl.unique());
        
    m_impl->update_glyphs = true;

    typedef UsedGlyphs::CodepointToGlyph::iterator Iterator;
    Iterator it = m_impl->codepoint_to_glyph.find(codepoint);
    if (it == m_impl->codepoint_to_glyph.end())
    {
        UInt16 glyph = m_face.codepoint_to_gid(codepoint);
        if (!glyph)
        {
            TRACE_WRN << "Codepoint " << codepoint
                      << "not found in " << m_face.family_name();
        }

        it = m_impl->codepoint_to_glyph.insert(
            std::make_pair(codepoint, glyph)).first;
    }

    return it->second;
}

void UsedGlyphs::add_glyph(UInt16 glyph)
{
    JAG_PRECONDITION(m_impl.unique());
    
    m_impl->update_map = true;
    m_impl->glyphs.insert(glyph);
}

void UsedGlyphs::copy_if_not_unique()
{
    if (!m_impl.unique())
        m_impl.reset(new Impl(*m_impl));
}

void UsedGlyphs::merge(UsedGlyphs const& other)
{
    JAG_ASSERT(m_impl.unique());
    
    if (m_impl->glyphs.empty() && m_impl->codepoint_to_glyph.empty())
    {
        // fast path
        JAG_ASSERT(&m_face == &other.m_face);
        m_impl = other.m_impl;
    }
    else
    {
        JAG_TO_BE_TESTED;

        Glyphs result;
        std::set_union(
            other.m_impl->glyphs.begin(),
            other.m_impl->glyphs.end(),
            m_impl->glyphs.begin(),
            m_impl->glyphs.end(),
            std::inserter(result, result.begin()));

        m_impl->glyphs.swap(result);

        typedef CodepointToGlyph::const_iterator MapIter;
        MapIter end = other.m_impl->codepoint_to_glyph.end();
        for(MapIter it = other.m_impl->codepoint_to_glyph.begin();
            it != end; ++it)
        {
            m_impl->codepoint_to_glyph.insert(*it);
        }

        if (other.m_impl->update_map)
            m_impl->update_map = true;

        if (other.m_impl->update_glyphs)
            m_impl->update_glyphs = true;
    }
}

void UsedGlyphs::update()
{
    if (m_impl->update_glyphs)
    {
        // update used glyphs by iterating the map and storing the values
        typedef CodepointToGlyph::const_iterator MapIter;
        MapIter end = m_impl->codepoint_to_glyph.end();
        for(MapIter it = m_impl->codepoint_to_glyph.begin();
            it != end; ++it)
        {
            m_impl->glyphs.insert(it->second);
        }

        m_impl->update_glyphs = false;
    }

    if (m_impl->update_map)
    {
        // update the map by finding codepoints for used glyphs
        typedef FaceCharIterator::Item Item;
        FaceCharIterator it = m_face.char_iterator();
        for(; !it.done(); it.next())
        {
            Item const& item = it.current();
            if (m_impl->glyphs.count(static_cast<UInt16>(item.glyph)))
            {
                m_impl->codepoint_to_glyph.insert(
                    std::make_pair(item.codepoint, item.glyph));
            }
        }
        m_impl->update_map = false;
    }
}

void UsedGlyphs::set_update_glyphs()
{
    JAG_ASSERT(m_impl.unique());
    m_impl->update_glyphs = true;
}

void UsedGlyphs::set_update_map()
{
    JAG_ASSERT(m_impl.unique());
    m_impl->update_map = true;
}


// ---------------------------------------------------------------------------
//                     class FaceCharIterator

FaceCharIterator::FaceCharIterator(FT_FaceRec_* face)
    : m_face(face)
{
    m_item.codepoint = FT_Get_First_Char(m_face, &m_item.glyph);
}

bool FaceCharIterator::done() const
{
    return m_item.glyph == 0;
}

FaceCharIterator::Item const& FaceCharIterator::current() const
{
    JAG_PRECONDITION(!done());
    return m_item;
}

void FaceCharIterator::next()
{
    m_item.codepoint = FT_Get_Next_Char(m_face,
                                        m_item.codepoint,
                                        &m_item.glyph);
}


} // namespace jag
