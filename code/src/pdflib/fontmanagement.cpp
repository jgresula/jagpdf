// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "fontmanagement.h"
#include "docwriterimpl.h"
#include "pdffont.h"
#include "fontdictionary.h"
#include "indirectobjectref.h"
#include "fontdescriptor.h"
#include "indirectobjectfromdirect.h"
#include "encdifferences.h"
#include "interfaces/directobject.h"
#include "objfmt.h"
#include <msg_pdflib.h>
#include <resources/typeman/typefaceutils.h>
#include <resources/interfaces/typeman.h>
#include <resources/interfaces/resourcectx.h>
#include <resources/interfaces/font.h>
#include <interfaces/execcontext.h>
#include <interfaces/configinternal.h>
#include <msg_resources.h>
#include <interfaces/configuration.h>

namespace jag {
namespace pdf {

//////////////////////////////////////////////////////////////////////////
FontManagement::FontManagement(DocWriterImpl& doc)
    : m_doc(doc)
    , m_fdict_id(0)
{
}


//
//
//
PDFFont const& FontManagement::font_load(Char const* fspec)
{
    // get a font from typeman by the spec
    ITypeMan& typeman(*m_doc.resource_ctx().type_man());
    IFontEx const& font = typeman.font_load(fspec, m_doc.exec_context());
    return font_load(font);
}

//
//
//
PDFFont const& FontManagement::font_load(IFontEx const& font)
{
    if (font.has_multiple_encondings())
    {
        std::auto_ptr<PDFFont> fnt(new PDFFont(*this, &font, 0));
        return lookup_font(fnt);
    }

    bool force_cid = false;
    switch (font.typeface().type())
    {
    case FACE_TRUE_TYPE:
        // the following block disallows using cid fonts for <1.3
        // that's correct for truetype (cannot be embedded as
        // CIDFontType2) but we need to reconsider this for other font
        // types once they are supported
        //
        // do not force cids version <1.3 - cannot embed the font program
        if (m_doc.version() >= 3)
            force_cid = static_cast<bool>(m_doc.exec_context().config().get_int("fonts.force_cid"));

        break;


    case FACE_TYPE_1_ADOBE_STANDARD:
        break;

    case FACE_OPEN_TYPE_CFF:
        // it seems that making CFF font by simply extracting cff
        // table works for versions < 1.6
//        m_doc.ensure_version(6, "OpenType with CFF");
        break;

    default:
        throw exception_invalid_input(msg_unknown_font_format()) << JAGLOC;
    }

    PDFFontDictData fdict_data(font, force_cid);
    // do not allow composite fonts for <1.3
    if (m_doc.version() < 3
         && PDFFontData::COMPOSITE_FONT == fdict_data.font_type())
    {
        throw exception_invalid_operation(msg_cid_fonts_not_supported()) << JAGLOC;
    }


    // retrieve font dictionary, it can already exist or we need to create a new
    // one
    FontDictMap::iterator fdict_it = m_fontdicts.find(fdict_data);
    if (fdict_it == m_fontdicts.end())
    {
        // create a new font dictionary and font and them to their maps
        // TBD: add one-based handle-id to FontDictMap ctor
        std::auto_ptr<FontDictionary> fdict(
            new FontDictionary(m_doc,
                               ++m_fdict_id,
                               fdict_data,
                               font.encoding_canonical()));

        std::auto_ptr<PDFFont> pdffont(new PDFFont(*this, &font, fdict.get()));
        PDFFont const& result = *pdffont;
        m_fonts.insert(pdffont.release());
        m_fontdicts.insert(fdict_data, fdict.release());

        return result;
    }
    else
    {
        // font dictionary already exists, create a pdf font a and try to look
        // it up in the font map
        std::auto_ptr<PDFFont> pdffont(
            new PDFFont(*this, &font, fdict_it->second));

        return lookup_font(pdffont);
    }
}

//
//
//
PDFFont const& FontManagement::lookup_font(std::auto_ptr<PDFFont>& pdffont)
{
    FontMap::iterator font_it = m_fonts.find(*pdffont);
    if (font_it == m_fonts.end())
    {
        PDFFont const& result = *pdffont;
        m_fonts.insert(pdffont.release());
        return result;
    }
    else
    {
        return *font_it;
    }
}

//
//
//
IndirectObjectRef FontManagement::font_ref(FontDictionary const& cfdict)
{
    // invoking this function inicates that someone needs this dictionary; we
    // need to store it so that it can be output when required
    FontDictionary& fdict = const_cast<FontDictionary&>(cfdict);
    FontsToOutput::iterator it = m_fonts_to_output.find(&fdict);
    if (it == m_fonts_to_output.end())
        m_fonts_to_output.insert(&fdict);

    return IndirectObjectRef(fdict);
}


//////////////////////////////////////////////////////////////////////////
/**
 * @brief Outputs used font descriptors and font dictionaries.
 *
 * Each pdf font dictionary references some font descriptor (basically
 * a typeface). Font descriptors might be shared among multiple
 * dictionaries. Font dictionaries with the same PDFFontData content
 * share the same font descriptors.
 */
void FontManagement::output_fonts()
{
    typedef std::map<PDFFontData,boost::shared_ptr<FontDescriptor> > FontDescriptors;
    FontDescriptors descriptors;

    // traverse through dictionaries, find a font descriptor for each and output it
    FontsToOutput::iterator end = m_fonts_to_output.end();
    for(FontsToOutput::iterator it=m_fonts_to_output.begin(); it!=end; ++it)
    {
        FontDictionary* dict = *it;
        PDFFontDictData const& dict_data(dict->fdict_data());

        if (dict_data.typeface().type()==FACE_TYPE_1_ADOBE_STANDARD)
        {
            // standard adobe T1 fonts do not need font descriptor until 1.5
            // as of now, we don't provide it even for >= 1.5

            // if the following assertion blows it indicates that font
            // type detection in PDFFontData ctor is not correct -
            // that might be case for encodings that need to be
            // written in form of differences
            JAG_ASSERT(PDFFontData::SIMPLE_FONT == dict_data.font_type());
        }
        else
        {
            // find out whether we already have a font descriptor for this dict,
            // otherwise create a new one
            PDFFontData const& font_data(dict_data.font_data());
            FontDescriptors::iterator desc_it = descriptors.find(font_data);
            if (desc_it == descriptors.end())
            {
                boost::shared_ptr<FontDescriptor> new_font_desc(new FontDescriptor(m_doc, font_data));
                desc_it = descriptors.insert(std::make_pair(font_data, new_font_desc)).first;
            }

            boost::shared_ptr<FontDescriptor> const& font_desc = desc_it->second;

            // in case of TrueType using GIDs we need to force font embedding
            // note: this action must be performed before we ask whether the
            // font program is going to be subset
            if (PDFFontData::COMPOSITE_FONT == dict_data.font_type()
                 && ENC_IDENTITY == dict_data.font_encoding())
            {
                ITypeface const& face(dict_data.typeface());
                if (FACE_TRUE_TYPE == face.type())
                    font_desc->force_embedding();
            }

            // in case the font program is going to be subset then report used codepoints
            if (font_desc->is_subset())
            {
                UsedGlyphs const& used_glyphs(dict->get_used_cids());
                font_desc->add_used_glyphs(used_glyphs);
            }

            dict->set_font_descriptor(font_desc);
        }

        dict->output_definition();
    }

    // output font descriptors
    FontDescriptors::iterator desc_end = descriptors.end();
    for(FontDescriptors::iterator desc_it=descriptors.begin(); desc_it!=desc_end; ++desc_it)
        (*desc_it->second).output_definition();
}

namespace
{
  // outputs encoding dictionary with differences
  class EncDiffDict
      : public IDirectObject
  {
      EnumCharacterEncoding m_enc;
  public:
      EncDiffDict(EnumCharacterEncoding enc) : m_enc(enc) {}
      void output_object(ObjFmt& fmt);
  };

  void EncDiffDict::output_object(ObjFmt& fmt)
  {
      Char const* enc_str = enc_differences(m_enc);
      fmt
          .dict_start()
          .dict_key("BaseEncoding").output("WinAnsiEncoding")
          .dict_key("Differences")
          .array_start()
          .raw_bytes(enc_str, strlen(enc_str))
          .array_end()
          .dict_end();
  }
}


IndirectObjectRef FontManagement::enc_diff_dict(EnumCharacterEncoding enc)
{
    JAG_PRECONDITION(supports_differences(enc));

    if (!m_enc_diffs)
        m_enc_diffs.reset(new IndirectObjectRef[ENC_NUM_ITEMS]);

    if (!is_valid(m_enc_diffs[enc]))
    {
        EncDiffDict enc_dict_writer(enc);
        IndirectObjectFromDirect enc_dict(m_doc,enc_dict_writer);
        enc_dict.output_definition();
        m_enc_diffs[enc] = IndirectObjectRef(enc_dict);
    }

    return m_enc_diffs[enc];
}


//////////////////////////////////////////////////////////////////////////
size_t FontManagement::dbg_num_fonts() const
{
    return m_fonts.size();
}


//////////////////////////////////////////////////////////////////////////
size_t FontManagement::dbg_num_dicts() const
{
    return m_fontdicts.size();
}


}} //namespace jag::pdf
