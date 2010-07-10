// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "fontdescriptor.h"
#include "genericcontentstream.h"
#include <core/generic/assert.h>
#include <core/generic/algorithms.h>
#include <core/generic/stringutils.h>
#include "objfmt.h"
#include <core/jstd/file_stream.h>
#include <core/jstd/streamhelpers.h>
#include <core/errlib/msg_writer.h>
#include <core/errlib/errlib.h>
#include <interfaces/configinternal.h>
#include <interfaces/execcontext.h>
#include <resources/typeman/typefaceutils.h>


#include <boost/shared_array.hpp>
#include <boost/bind.hpp>



/*
- the units of glyph space are one-thousandth of a unit of text space
  1/(72*1000) dpi

A font defines the glyphs for one standard size. This standard is arranged so that
the nominal height of tightly spaced lines of text is 1 unit. In the default user
coordinate system, this means the standard glyph size is 1 unit in user space, or
1/72 inch.
*/

using namespace jag::jstd;

namespace jag {
namespace pdf {

namespace
{
  /// handler writing a stream with a TrueType CFF font program.
  void embed_tt_stream(ISeqStreamInput& in_stream, ObjFmt& writer)
  {
      writer
          .dict_key("Length1")
          .space()
          .output(static_cast<UInt> (in_stream.tell()))
      ;
  }



  /// handler writing a stream with a OpenType CFF font program.
  void embedded_cff_stream(Char const* subtype, ObjFmt& writer)
  {
      writer.dict_key("Subtype").output(subtype);
  }

  /// Values for FontStrech field
  char const* const s_font_stretch_str[10] =
  {
      "", "UltraCondensed", "ExtraCondensed", "Condensed", "SemiCondensed",
      "Normal", "SemiExpanded", "Expanded", "ExtraExpanded", "UltraExpanded"
  };


  /// Bits in Flags field
  enum FontDescriptorFlags
  {
      FDF_FIXED_PITCH  = 1 << 0,
      FDF_SERIF_GLYPHS = 1 << 1,
      FDF_SYMBOLIC     = 1 << 2,
      FDF_SCRIPT       = 1 << 3,
      FDF_NONSYMBOLIC  = 1 << 5,
      FDF_ITALIC       = 1 << 6,
      FDF_ALLCAP       = 1 << 16,
      FDF_SMALLCAP     = 1 << 17,
      FDF_FORCE_BOLD   = 1 << 18
  };


  template<class W>
  IndirectObjectRef output_embedded_font(DocWriterImpl& doc, IStreamInput& font_program, W writer)
  {
      GenericContentStream cs(doc, writer);
      jstd::copy_stream(font_program, cs.out_stream());
      cs.output_definition();
      return IndirectObjectRef(cs);
  }

} // anonymous namespace



//////////////////////////////////////////////////////////////////////////
FontDescriptor::FontDescriptor(DocWriterImpl& doc, PDFFontData const& font_data)
    : IndirectObjectImpl(doc)
    , m_font_data(font_data)
    , m_force_embedding(false)
    , m_used_glyphs(font_data.typeface())
    , m_method_depending_on_force_embedding_flag_invoked(false)
    , m_embedded_regardless_of_copyright(false)
{
}



/// Retrieves font base name as required by fields of font descriptor and font dictionary.
char const* FontDescriptor::basename() const
{
    // Test cases:
    //  - font without postscript_name
    //  - subset_font
    //  - synthesized font
    //JAG_TO_BE_TESTED;

    // The name is constructed on the first call and then stored in m_basename

    m_method_depending_on_force_embedding_flag_invoked = true;

    if (m_basename.empty())
    {
        m_basename = m_font_data.typeface().postscript_name();
        if (m_basename.empty())
        {
            // e.g. TrueType might have postcript name empty
            // in such case facename with removed spaces is used
            Char const*const family_name = m_font_data.typeface().family_name();
            JAG_ASSERT_MSG(!is_empty(family_name), "that should be ensured by type manager");
            const size_t len = strlen(family_name);
            boost::shared_array<Char> fontname(new Char[len]);

            Char* fontname_end = copy_if(
                family_name
                , family_name+len
                , fontname.get()
                , std::unary_negate<fn_isspace>(fn_isspace()));

            std::string(fontname.get(), fontname_end).swap(m_basename);
        }

        if (is_subset())
        {
            // For a font subset, the PostScript name of the font-the value of
            // the font's BaseFont entry and the font descriptor's FontName
            // entry-begins with a tag followed by a plus sign (+). The tag
            // consists of exactly six uppercase letters; the choice of letters
            // is arbitrary, but different subsets in the same PDF file must
            // have different tags. For example, EOODIA+Poetica is the name of a
            // subset of Poetica®, a Type 1 font. (See implementation note 62 in
            // Appendix H.)
            const size_t subset_prefix_len = 7;
            Char subset_prefix[subset_prefix_len];
            for(int i=0; i<6; ++i)
                subset_prefix[i] = static_cast<char>(rand()%('Z'-'A'+1) + 'A');
            subset_prefix[6]='+';

            std::string subset_basename(subset_prefix, subset_prefix+subset_prefix_len);
            subset_basename += m_basename;
            m_basename.swap(subset_basename);
        }

        {
            // If the font in a source document uses a bold or italic style but
            // there is no font data for that style, the host operating system
            // synthesizes the style. In this case, a comma and the style name
            // (one of Bold, Italic, or BoldItalic) are appended to the font
            // name. For example, for a TrueType font that is a bold variant of
            // the New York font, the BaseFont value is written as /NewYork,Bold
            std::string synthesized_part;
            if (m_font_data.synthesized_bold())
                synthesized_part += "Bold";
            if (m_font_data.synthesized_italic())
                synthesized_part += "Italic";
            if (!synthesized_part.empty())
            {
                std::string synthesized(m_basename);
                synthesized += ',';
                synthesized += synthesized_part;
                m_basename.swap(synthesized);
            }
        }
    }

    return m_basename.c_str();
}



/**
 * @brief Indicates that the font program is going to be subset.
 *
 * Subset is done only when the font program is embedded.
 */
bool FontDescriptor::is_subset() const
{
    // Test cases:
    //  - forbid/allow subsetting in configuration
    //  - forbid/allow embedding in configuration
    //JAG_TO_BE_TESTED;

    m_method_depending_on_force_embedding_flag_invoked = true;

    if (!doc().exec_context().config().get_int("fonts.subset"))
        return false;

    if (!is_embedded())
        return false;

    ITypeface const& typeface(m_font_data.typeface());
    return typeface.can_subset() ? true : false;
}



/// Indicates whether the font program is going to be embedded.
bool FontDescriptor::is_embedded() const
{
    // Test cases:
    //  - forbid/allow embedding in configuration
    //  - force embedding
    //JAG_TO_BE_TESTED;

    m_method_depending_on_force_embedding_flag_invoked = true;
    bool embed = m_force_embedding || (doc().exec_context().config().get_int("fonts.embedded") ? true : false);

    ITypeface const& typeface(m_font_data.typeface());

    // does font copyright allow to embed this file?
    if (embed && !typeface.can_embed())
    {
        m_embedded_regardless_of_copyright = true;
        // an alternative would be to throw an exception
    }

    return embed;
}



/**
 * @brief Some clients might enforce font embedding (required by PDF spec/recommendation).
 *
 * Ideally, this should be set in ctor as other methods depends
 * on it - basename(), is_subset(), is_embedded(). Currently
 * FontDescriptor is being created before this information is known.
 *
 */
void FontDescriptor::force_embedding()
{
    JAG_PRECONDITION(!m_method_depending_on_force_embedding_flag_invoked);
    m_force_embedding = true;
}



//////////////////////////////////////////////////////////////////////////
void FontDescriptor::add_used_glyphs(UsedGlyphs const& used_glyphs)
{
    m_used_glyphs.merge(used_glyphs);
}



/**
 * @brief Handles font file.
 *
 * In case the associated font program is going to be embedded this action
 * is triggered here.
 *
 * Currently font programs having single stream are supported. Things might
 * get more complicated in the future as some fonts comprise of more then
 * one stream.
 *
 * @todo issue warning when we are forced to embed the font font file
 *       regardless of its copyright
 */
bool FontDescriptor::on_before_output_definition()
{
    if (!is_embedded())
        return true;

    ITypeface const& typeface(m_font_data.typeface());

    // we need to get the font program stream (or possibly its subset)
    std::auto_ptr<IStreamInput> font_program;
    if (is_subset() && typeface.can_subset())
    {
        unsigned subset_options = 0;
#if 0
        // spec (5.8) says that embedded TrueType CID fonts do not need cmap but
        // without it does not work in acrobat, (also it might be some problem
        // in the subsetting code)
        if (ITypeface::TRUE_TYPE==typeface->type() &&
            PDFFontData::COMPOSITE_FONT == m_font_data.font_type())
        {
            subset_options |= ITypeface::DONT_INCLUDE_CMAP;
        }
#endif

        m_used_glyphs.update();
        font_program =
            typeface.subset_font_program(m_used_glyphs, subset_options);
    }
    else
    {
        // Note: it would be possible to drop some tables here but is
        // questionable wheter it does not degrade quality of the rendered fonts

        unsigned options = 0;
        // see #98 - always extract CFF instead of using whole OpenType file
        if (FACE_OPEN_TYPE_CFF==typeface.type())
        {
            // extract CFF as version <1.6 is being produced
            options |= ITypeface::EXTRACT_CFF;
        }
        font_program = typeface.font_program(0, options);
    }

#if 0
    // outputs font for debugging purposes
    {
        static int i=0;
        char fname[128];
        sprintf(fname, "/tmp/pdf_%d_%s.ttf", ++i, basename());
        FileStreamOutput fout(fname);
        copy_stream(*font_program, fout);
        font_program->seek(0, OFFSET_FROM_BEGINNING);
    }
#endif

    if (FACE_TRUE_TYPE == typeface.type())
    {
        m_font_file2 = output_embedded_font(
            doc(),
            *font_program,
            boost::bind(embed_tt_stream, boost::ref(*font_program), _1));
    }
    else if (FACE_OPEN_TYPE_CFF == typeface.type())
    {
        Char const* subtype =
            (PDFFontData::COMPOSITE_FONT==m_font_data.font_type())
            ? "CIDFontType0C"
            : "Type1C";


        m_font_file3 = output_embedded_font(
            doc(),
            *font_program,
            boost::bind(embedded_cff_stream, subtype, _1));
    }
    else
    {
        // attemp to embed unsupported format, should be catched earlier
        JAG_INTERNAL_ERROR;
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////
void FontDescriptor::on_output_definition()
{
    JAG_PRECONDITION(!is_subset() || !m_used_glyphs.glyphs().empty());

    ObjFmt& writer = object_writer();
    ITypeface const& face(m_font_data.typeface());
    TypefaceMetrics const&  metrics(face.metrics());
    resources::FontSpaceToGlyphSpace fs2gs(metrics);
    unsigned int flags = face_flags();

    if (m_embedded_regardless_of_copyright)
        write_message(WRN_FORCED_EMBEDDING_w, face.full_name().c_str());

    writer
        .dict_start()
        .dict_key("Type").output("FontDescriptor")
        .dict_key("FontName").name(basename())
        .dict_key("Flags").space().output(flags)
    .dict_key("FontBBox")
          .array_start()
            .output(fs2gs(metrics.bbox_xmin)).space()
            .output(fs2gs(metrics.bbox_ymin)).space()
            .output(fs2gs(metrics.bbox_xmax)).space()
            .output(fs2gs(metrics.bbox_ymax))
          .array_end()
        .dict_key("ItalicAngle").space().output(round(face.italic_angle()))
    .dict_key("Ascent").space().output(fs2gs(metrics.ascent))
    .dict_key("Descent").space().output(fs2gs(metrics.descent))
    .dict_key("AvgWidth").space().output(fs2gs(metrics.avg_width))
    .dict_key("MaxWidth").space().output(fs2gs(metrics.max_width))
     ;

    double stem_subval = face.weight_class()/65.0;
    writer.dict_key("StemV").space().output(50 + int(stem_subval*stem_subval));

    if (is_valid(m_font_file2))
    {
        JAG_ASSERT(!is_valid(m_font_file3));
        writer.dict_key("FontFile2").space().ref(m_font_file2);
    }
    else if (is_valid(m_font_file3))
    {
        writer.dict_key("FontFile3").space().ref(m_font_file3);
    }


    if (metrics.xheight)
        writer.dict_key("XHeight").space().output(fs2gs(metrics.xheight));

    if (flags & FDF_NONSYMBOLIC)
    writer.dict_key("CapHeight").space().output(fs2gs(metrics.cap_height));

    // some of the fields only in version 1.5+
    if (5 <= doc().version())
    {
        writer
            .dict_key("FontFamily").text_string(face.family_name())
            .dict_key("FontWeight").space().output(face.weight_class())
        ;

        Int width_class = face.width_class();
        if (width_class > 0 && width_class < 10)
        {
            writer
                .dict_key("FontStretch")
                .output(s_font_stretch_str[width_class])
            ;
        }
    }

    // Leading, StemH and MissingWidth not provided
    writer.dict_end();
}

/**
 * @brief Enumerates PANOSE offsets
 */

/// Sets up Flags field.
unsigned int FontDescriptor::face_flags()
{
    enum PanoseOffsets
    {
        FAMILY_TYPE
        , SERIF_STYLE
        , WEIGHT
        , PROPORTION
        , CONTRAST
        , STROKE_VARIATION
        , ARM_STYLE
        , LETTERFORM
        , MIDLINE
        , XHEIGHT
    };

    unsigned int flags = 0;
    ITypeface const& face = m_font_data.typeface();
    Panose const& panose(face.panose());


    switch (panose[FAMILY_TYPE])
    {
    case 2: //latin text
        flags |= FDF_NONSYMBOLIC;
        break;

    case 3: //latin hand written
        flags |= FDF_SCRIPT | FDF_NONSYMBOLIC;
        break;

    case 4: // decorative
        flags |= FDF_NONSYMBOLIC; // tbd: where to get allcap, smallcap;
        break;

    case 5: // symbol
        flags |= FDF_SYMBOLIC;
    }

    // sans serif (=no serif) when it is one of Flared, Rounded, Perpendicular, Sans Serif, Obtuse, Normal
    // according to http://www.monotypeimaging.com/printer/pan2.asp
    if (panose[SERIF_STYLE] > 1 && panose[SERIF_STYLE] < 11) //no-fit or any
        flags |= FDF_SERIF_GLYPHS;

    flags |= face.fixed_width() ? FDF_FIXED_PITCH : 0;
    flags |= face.bold() ? FDF_FORCE_BOLD : 0;
    flags |= face.italic() ? FDF_ITALIC : 0;

    return flags;
}

}} //namespace jag::pdf


