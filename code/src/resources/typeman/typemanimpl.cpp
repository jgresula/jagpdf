// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <resources/typeman/typemanimpl.h>
//#include "typefacefactory.h"
#include "fontspecimpl.h"
#include "systemfontmapping.h"
#include "freetypeopenargs.h"
#include "t1adobestandardface.h"
#include <core/generic/checked_cast.h>
#include <core/generic/refcountedimpl.h>
#include <core/generic/stringutils.h>
#include <core/errlib/msg_writer.h>
#include <core/errlib/errlib.h>
#include <core/jstd/optionsparser.h>
#include <core/jstd/crt_platform.h>
#include <core/jstd/unicode.h>
#include <resources/typeman/charencrecord.h>
#include <resources/typeman/typefaceimpl.h>
#include <msg_resources.h>
#include <core/jstd/fileso.h>
#include <boost/spirit/include/classic_symbols.hpp>
#include <boost/ref.hpp>
#include <interfaces/configinternal.h>
#include <interfaces/execcontext.h>

using namespace jag::jstd;
using namespace boost;

namespace jag {
namespace resources {

namespace
{
  //fwd
  CharEncodingRecord const& get_text_encoding_rec(Char const* encoding,
                                                   ITypeface const& face,
                                                   FontSpecImpl const& spec);
  CharEncodingRecord const& find_encoding_record(Char const* encoding);
}

//////////////////////////////////////////////////////////////////////////
TypeManImpl::TypeManImpl()
{
    FT_Library ft_library;
    FT_Error err = FT_Init_FreeType(&ft_library);
    if (err) {
        JAG_INTERNAL_ERROR_MSG_CODE("FreeType initialization failed", err);
    }

    m_ft_library = shared_ptr<FT_LibraryRec_>(ft_library, &FT_Done_FreeType);
}


/////////////////////////////////////////////////////////////////////////
intrusive_ptr<IFontSpec> TypeManImpl::define_font() const
{
    return intrusive_ptr<IFontSpec>(new RefCountImpl<FontSpecImpl>());
}


//////////////////////////////////////////////////////////////////////////
namespace
{
  enum FontSpecKeywords {
      FACENAME, POINTSIZE, FONTFILE, ENCODING
  };


  struct OptionKeywords
      : public spirit::classic::symbols<unsigned>
  {
      OptionKeywords()
      {
          add
              ("name", FACENAME)
              ("size", POINTSIZE)
              ("enc", ENCODING)
              ("file", FONTFILE)
              ;
      }
  } g_option_keywords;


  enum FontSpecValues {
      ITALIC, BOLD, ADOBE14
  };


  struct OptionValues
      : public spirit::classic::symbols<unsigned>
  {
      OptionValues()
      {
          add
              ("italic", ITALIC)
              ("bold", BOLD)
              ("standard", ADOBE14)
              ;
      }
  } g_option_values;


  ///
  /// Creates a font spec from a string.
  ///
  /// If an existing spec is passed then it is filled in instead of
  /// creating a new one.
  ///
  intrusive_ptr<FontSpecImpl>
  spec_from_string(Char const* strspec,
                    intrusive_ptr<FontSpecImpl> spec_= intrusive_ptr<FontSpecImpl>())
  {
      intrusive_ptr<FontSpecImpl> fspec;
      if (spec_)
      {
          fspec = spec_;
      }
      else
      {
          fspec = new RefCountImpl<FontSpecImpl>();
      }

      try
      {
          ParsedResult const& p =
              parse_options(strspec, ParseArgs(&g_option_keywords, &g_option_values));

          fspec->filename(p.to_<string_32_cow>(FONTFILE).c_str());
          fspec->facename(p.to_<string_32_cow>(FACENAME).c_str());
          fspec->encoding(p.to_<string_32_cow>(ENCODING).c_str());
          fspec->size(p.to_<double>(POINTSIZE, 0.0));
          fspec->bold(p.has_explicit_value(BOLD));
          fspec->italic(p.has_explicit_value(ITALIC));
          fspec->adobe14(p.has_explicit_value(ADOBE14));
      }
      catch(exception const& exc)
      {
          throw exception_invalid_value(msg_invalid_font_spec_str(), &exc) << JAGLOC;
      }

      return fspec;
  }

} // anonymous namespace


///
/// Registers a font specified by a string.
///
IFontEx const&
TypeManImpl::font_load(char const* spec, IExecContext const& exec_ctx)
{
    return font_load_main(spec_from_string(spec), exec_ctx);
}

///
/// Registers a font specified by a spec object.
///
IFontEx const&
TypeManImpl::font_load_spec(intrusive_ptr<IFontSpec> const& spec,
                                            IExecContext const& exec_ctx)
{
    return font_load_main(spec, exec_ctx);
}


///
/// Font registration internal entry point.
///
/// First it tries to register font with the given font spec. If it
/// fails then it attempts to create the default font in case the
/// client specified any.
///
IFontEx const&
TypeManImpl::font_load_main(intrusive_ptr<IFontSpec> const spec,
                             IExecContext const& exec_ctx)
{
    JAG_PRECONDITION(spec);
    FontSpecImpl const& specimpl = *checked_static_cast<FontSpecImpl const*>(spec.get());
    specimpl.ensure_consistency();

    try
    {
        return font_load_internal(specimpl, exec_ctx);
    }
    catch(exception const& exc)
    {
        // default font is not used for adobe core fonts as such
        // requests must alway succeed (if correct)
        if (specimpl.adobe14())
            throw;

        // now, find out if the client set a default font, if so then
        // clone the spec fill it in from the default font from the
        // configuration string and try to create a font from it
        Char const* default_font = exec_ctx.config().get("fonts.fallback");
        if (is_empty(default_font))
            throw;

        if (specimpl.defined_by_filename())
            write_message(WRN_FALLBACK_FONT_USED_INSTEAD_w, specimpl.filename());
        else
            write_message(WRN_FALLBACK_FONT_USED_INSTEAD_w, specimpl.fullname().c_str());

        try
        {
            intrusive_ptr<FontSpecImpl> spec_copy(specimpl.clone());
            spec_copy = spec_from_string(default_font, spec_copy);
            return font_load_internal(*spec_copy, exec_ctx);
        }
        catch(exception& default_font_exc)
        {
            // even the default font creation failed, throw a chained
            // exception
            default_font_exc.set_next(exc);
            throw exception_operation_failed(msg_default_typeface_not_found(),
                                              &default_font_exc) << JAGLOC;
        }
    }
}

///
/// Registers font specification and gives back an associated handle.
///
IFontEx const&
TypeManImpl::font_load_internal(FontSpecImpl const& specimpl,
                                 IExecContext const& exec_ctx)
{
    // Create a typeface object based on font spec. The object is
    // always instantiated, but might be thrown away later on if there
    // is already such typeface registered.
    //
    // We need to instantiate it always to be able to calculate its
    // hash value (the hash value can comprise e.g. of a portion of
    // typeface stream) and match it with already registered ones.

    CharEncodingRecord const& required_enc(
        find_encoding_record(specimpl.encoding()));

    std::auto_ptr<ITypeface> typeface_ptr;
    if (specimpl.adobe14())
    {
        if (required_enc.encoding == ENC_UTF_8)
            return create_adobe14_multienc_font(specimpl, exec_ctx);
        else
            typeface_ptr = create_typeface_adobe14(specimpl);
    }
    else
    {
        typeface_ptr = specimpl.defined_by_filename()
            ? create_typeface_from_file(specimpl, exec_ctx, required_enc)
            : create_typeface_by_search(specimpl, exec_ctx, required_enc);

        // client might want to disable synthesized fonts
        check_synthesized_font(specimpl, *typeface_ptr, exec_ctx);
    }
    JAG_ASSERT(typeface_ptr.get());

    Typefaces::iterator face_it = m_typefaces.find(*typeface_ptr);
    if (face_it == m_typefaces.end())
        face_it = m_typefaces.insert(typeface_ptr.release()).first;

    // create a font object and find out whether it is not already
    // present in our m_fonts_map, if so just return the associated
    // handle, otherwise insert it to the resource table and the map
    CharEncodingRecord const& text_enc_rec(
        get_text_encoding_rec(specimpl.encoding(), *face_it, specimpl));

    std::auto_ptr<FontImpl> font(
        new FontImpl(specimpl, *face_it, text_enc_rec, exec_ctx));
    
    return lookup_font(font, m_fonts_map);
}


//
//
//
template<class FONT>
IFontEx const& TypeManImpl::lookup_font(
    std::auto_ptr<FONT>& font,
    std::set<boost::reference_wrapper<FONT> >& map)
{
    typedef std::set<boost::reference_wrapper<FONT> > Map;

    typename Map::iterator f_it = map.find(boost::ref(*font));
    if (f_it == map.end())
    {
        // a new font
        FONT* font_raw = font.get();
        map.insert(boost::ref(*font));
        m_fonts_storage.push_back(font.release());
        return *font_raw;
    }
    else
    {
        // font already exists
        return *f_it;
    }
}



///
/// Try to load the typeface from a file specified in fontspec.
///
std::auto_ptr<ITypeface>
TypeManImpl::create_typeface_from_file(FontSpecImpl const& spec,
                                       IExecContext const& /*ctx*/,
                                       CharEncodingRecord const& /*enc_rec*/) const
{
    std::auto_ptr<FTOpenArgs> ft_args(new FTOpenArgs(spec.filename()));
    return std::auto_ptr<ITypeface>(new TypefaceImpl(m_ft_library, ft_args));
}



///
/// Try to load the typeface using typeface matching.
///
std::auto_ptr<ITypeface>
TypeManImpl::create_typeface_by_search(FontSpecImpl const& spec,
                                       IExecContext const& /*exec_ctx*/,
                                       CharEncodingRecord const& enc_rec) const
{
    // let the system find the font by name
    std::auto_ptr<ITypeface> typeface(
        typeface_from_system(m_ft_library, &spec, enc_rec));

    if (typeface.get())
        return typeface;

    throw exception_operation_failed(
        msg_cannot_find_font(spec.fullname().c_str())) << JAGLOC;
}

///
/// Creates a typeface representing an adobe core font.
///
std::auto_ptr<ITypeface>
TypeManImpl::create_typeface_adobe14(FontSpecImpl const& spec) const
{
    return std::auto_ptr<ITypeface>(new T1AdobeStandardFace(spec));
}


namespace
{
  // The following list of encodings was obtained from analysis of adobe core 14
  // .afm files.
  //
  // There are still 88+ codepoints (mainly symbols) defined in .afm that are not
  // present in any of these encodings. The reason is that the 'unicode ->
  // codepage' mapping algorithm uses only ICU encodings and the mentioned
  // symbols are not present in any of ICU encodings.
  //
  const int NUM_ENC14 = 9;
  EnumCharacterEncoding g_enc14[NUM_ENC14] = {
      ENC_ISO_8859_1,
      ENC_ISO_8859_2,
      ENC_MAC_ROMAN,
      ENC_ISO_8859_4,
      ENC_ISO_8859_7,
      ENC_ISO_8859_3,
      ENC_ISO_8859_13,
//      ENC_ISO_8859_16,
      ENC_ISO_8859_15,
      ENC_ISO_8859_14
  };
}

//
//
//
IFontEx const& TypeManImpl::create_adobe14_multienc_font(
    FontSpecImpl const& spec,
    IExecContext const& exec_ctx)
{
    std::auto_ptr<MultiEncFontImpl> font(
        new MultiEncFontImpl(*this, exec_ctx, spec,
                             g_enc14, g_enc14 + NUM_ENC14));

    return lookup_font(font, m_multi_enc_fonts_map);
}


//
//
//
void TypeManImpl::check_synthesized_font(FontSpecImpl const& fspec,
                                          ITypeface& typeface,
                                          IExecContext const& exec_ctx) const
{
    // ? is it possible that there can be e.g. only bold but not regular typeface
    if (!exec_ctx.config().get_int("fonts.synthesized"))
    {
        // synthesized fonts not allowed, check typeface
        if (fspec.bold()!=typeface.bold() || fspec.italic()!=typeface.italic())
        {
            std::string name(typeface.family_name());
            if (fspec.bold())
                name += " Bold";

            if (fspec.italic())
                name += " Italic";

            throw exception_operation_failed(msg_cannot_find_font(name.c_str())) << JAGLOC;
        }
    }
}



//////////////////////////////////////////////////////////////////////////
ULong TypeManImpl::dbg_num_typefaces() const
{
    return m_typefaces.size();
}

//////////////////////////////////////////////////////////////////////////
void TypeManImpl::dbg_dump_typefaces() const
{
    for(
          Typefaces::const_iterator it = m_typefaces.begin()
        ; it != m_typefaces.end()
        ; ++it
   )
    {
        printf("%s %s\n", (*it).family_name(), (*it).style_name());
    }
}

//////////////////////////////////////////////////////////////////////////
ULong TypeManImpl::dbg_num_fonts() const
{
    return m_fonts_map.size();
}


//
//
//

namespace
{
  /// symbol table that maps cannonized icu converter names to encoding records
  struct Encodings
      : public spirit::classic::symbols<CharEncodingRecord>
  {
      Encodings()
      {
          add
              // http://demo.icu-project.org/icu-bin/convexp?conv=ibm-5346_P100-1998&s=ALL
              ("ibm-5346_P100-1998", CharEncodingRecord(ENC_CP_1250, "ibm-5346_P100-1998", 238 /*EASTEUROPE_CHARSET*/))

              // http://demo.icu-project.org/icu-bin/convexp?conv=ibm-5347_P100-1998&s=ALL
              ("ibm-5347_P100-1998", CharEncodingRecord(ENC_CP_1251, "ibm-5347_P100-1998", 204 /*RUSSIAN_CHARSET*/))

              // http://demo.icu-project.org/icu-bin/convexp?conv=ibm-5348_P100-1997&s=ALL
              ("ibm-5348_P100-1997", CharEncodingRecord(ENC_CP_1252, "ibm-5348_P100-1997", 0   /*ANSI_CHARSET*/))

              // http://demo.icu-project.org/icu-bin/convexp?conv=ibm-5349_P100-1998&s=ALL
              ("ibm-5349_P100-1998", CharEncodingRecord(ENC_CP_1253, "ibm-5349_P100-1998", 161 /*GREEK_CHARSET*/))

              // http://demo.icu-project.org/icu-bin/convexp?conv=ibm-5350_P100-1998&s=ALL
              ("ibm-5350_P100-1998", CharEncodingRecord(ENC_CP_1254, "ibm-5350_P100-1998", 162 /*TURKISH_CHARSET*/))

              // http://demo.icu-project.org/icu-bin/convexp?conv=ibm-9447_P100-2002&s=ALL
              ("ibm-9447_P100-2002", CharEncodingRecord(ENC_CP_1255, "ibm-9447_P100-2002", 177 /*HEBREW_CHARSET*/))

              // http://demo.icu-project.org/icu-bin/convexp?conv=ibm-9448_X100-2005&s=ALL
              ("ibm-9448_X100-2005", CharEncodingRecord(ENC_CP_1256, "ibm-9448_X100-2005", 178 /*ARABIC_CHARSET*/))

              // http://demo.icu-project.org/icu-bin/convexp?conv=ibm-9449_P100-2002&s=ALL
              ("ibm-9449_P100-2002", CharEncodingRecord(ENC_CP_1257, "ibm-9449_P100-2002", 186 /*BALTIC_CHARSET*/))

              // http://demo.icu-project.org/icu-bin/convexp?conv=ibm-5354_P100-1998&s=ALL
              ("ibm-5354_P100-1998", CharEncodingRecord(ENC_CP_1258, "ibm-5354_P100-1998", 163 /*VIETNAMESE_CHARSET*/))

              // http://demo.icu-project.org/icu-bin/convexp?conv=macos-0_2-10.2&s=ALL
              ("macos-0_2-10.2",    CharEncodingRecord(ENC_MAC_ROMAN, "macos-0_2-10.2",  77  /*MAC_CHARSET*/))

              // http://demo.icu-project.org/icu-bin/convexp?conv=ibm-1276_P100-1995&s=ALL
              ("ibm-1276_P100-1995", CharEncodingRecord(ENC_PDF_STANDARD, "ibm-1276_P100-1995"))

              // http://demo.icu-project.org/icu-bin/convexp?conv=ISO-8859-1&s=ALL
              ("ISO-8859-1", CharEncodingRecord(ENC_ISO_8859_1, "ISO-8859-1"))

              // http://demo.icu-project.org/icu-bin/convexp?conv=ibm-912_P100-1995&s=ALL
              ("ibm-912_P100-1995", CharEncodingRecord(ENC_ISO_8859_2, "ibm-912_P100-1995"))

              // http://demo.icu-project.org/icu-bin/convexp?conv=ibm-913_P100-2000&s=ALL
              ("ibm-913_P100-2000", CharEncodingRecord(ENC_ISO_8859_3, "ibm-913_P100-2000"))

              // http://demo.icu-project.org/icu-bin/convexp?conv=ibm-914_P100-1995&s=ALL
              ("ibm-914_P100-1995", CharEncodingRecord(ENC_ISO_8859_4, "ibm-914_P100-1995"))

              // http://demo.icu-project.org/icu-bin/convexp?conv=ibm-915_P100-1995&s=ALL
              ("ibm-915_P100-1995", CharEncodingRecord(ENC_ISO_8859_5, "ibm-915_P100-1995"))

              // http://demo.icu-project.org/icu-bin/convexp?conv=ibm-1089_P100-1995&s=ALL
              ("ibm-1089_P100-1995", CharEncodingRecord(ENC_ISO_8859_6, "ibm-1089_P100-1995"))

              // http://demo.icu-project.org/icu-bin/convexp?conv=ibm-9005_X110-2007&s=ALL
              ("ibm-9005_X110-2007", CharEncodingRecord(ENC_ISO_8859_7, "ibm-9005_X110-2007"))

              // http://demo.icu-project.org/icu-bin/convexp?conv=ibm-5012_P100-1999&s=ALL
              ("ibm-5012_P100-1999", CharEncodingRecord(ENC_ISO_8859_8, "ibm-5012_P100-1999"))

              // http://demo.icu-project.org/icu-bin/convexp?conv=ibm-920_P100-1995&s=ALL
              ("ibm-920_P100-1995", CharEncodingRecord(ENC_ISO_8859_9, "ibm-920_P100-1995"))

              // http://demo.icu-project.org/icu-bin/convexp?conv=iso-8859_10-1998&s=ALL
              ("iso-8859_10-1998", CharEncodingRecord(ENC_ISO_8859_10,"iso-8859_10-1998"))

              // Note: no IANA name for 8859-11 - not documented
              // http://demo.icu-project.org/icu-bin/convexp?conv=iso-8859_11-2001&s=ALL
              ("iso-8859_11-2001", CharEncodingRecord(ENC_ISO_8859_11,"iso-8859_11-2001"))

              // http://demo.icu-project.org/icu-bin/convexp?conv=ibm-921_P100-1995&s=ALL
              ("ibm-921_P100-1995", CharEncodingRecord(ENC_ISO_8859_13,"ibm-921_P100-1995"))

              // http://demo.icu-project.org/icu-bin/convexp?conv=iso-8859_14-1998&s=ALL
              ("iso-8859_14-1998", CharEncodingRecord(ENC_ISO_8859_14,"iso-8859_14-1998"))

              // http://demo.icu-project.org/icu-bin/convexp?conv=ibm-923_P100-1998&s=ALL
              ("ibm-923_P100-1998", CharEncodingRecord(ENC_ISO_8859_15,"ibm-923_P100-1998"))

// not part of the standard ICU distribution
              // http://demo.icu-project.org/icu-bin/convexp?conv=iso-8859_16-2001&s=ALL
//              ("iso-8859_16-2001", CharEncodingRecord(ENC_ISO_8859_16,"iso-8859_16-2001"))


              // Unicode & TrueType
              //
              // Only TrueType with Unicode encoding is supported - see
              // TypefaceImpl::detect_type(). Thus, we allow specifying a
              // Unicode encoding for Win font matching. There is nothing like
              // UNICODE_CHARSET that could be passed to LOGFONT::lfCharSet so
              // we use ANSI_CHARSET instead.

              // http://demo.icu-project.org/icu-bin/convexp?conv=UTF-8&s=ALL
              ("UTF-8",           CharEncodingRecord(ENC_UTF_8, "UTF-8", 0   /*ANSI_CHARSET*/))

// 16-bits unicode encodings disabled for now - text methods take char
// const* argument
//               ("UTF-16",          CharEncodingRecord(ENC_UTF_16, "UTF-16"))
//               ("UTF-16BE",        CharEncodingRecord(ENC_UTF_16BE, "UTF-16BE"))
//               ("UTF-16LE",        CharEncodingRecord(ENC_UTF_16LE, "UTF-16LE"))
              ("builtin",         CharEncodingRecord(ENC_BUILTIN, ""))


              //win ext:Windows-31J / shift_jis       SHIFTJIS_CHARSET    cp 932
              ("shift_jis",      CharEncodingRecord(ENC_SHIFT_JIS,  "shift_jis",        128 /*SHIFTJIS_CHARSET*/))
              // GB_2312-80
              ("gb2312",         CharEncodingRecord(ENC_GB2312,     "gb2312",           134 /*GB2312_CHARSET*/))
              ("ks_c_5601-1987", CharEncodingRecord(ENC_HANGEUL,    "ks_c_5601-1987",   129 /*HANGEUL_CHARSET*/))
              ("big5",           CharEncodingRecord(ENC_BIG5,       "big5",             136 /*CHINESEBIG5_CHARSET*/))

              ;
      }
  } g_encodings;


  ///
  /// Determine text encoding by consulting encoding name and the face encoding scheme.
  ///
  CharEncodingRecord const& get_text_encoding_rec(Char const* encoding,
                                                   ITypeface const& face,
                                                   FontSpecImpl const& spec)
  {
      CharEncodingRecord const& encrec = find_encoding_record(encoding);

      if (spec.adobe14() && encrec.encoding==ENC_UTF_8)
          throw exception_invalid_value(msg_core_font_with_unicode_enc()) << JAGLOC;

      // we are done if encoding other than built-in is specified
      if (encrec.encoding != ENC_BUILTIN)
          return encrec;

      Char const*const enc_scheme = face.encoding_scheme();
      if (is_empty(enc_scheme))
      {
          // no internal encoding scheme, set a default one - that
          // might differ on other platforms
          return find_encoding_record("windows-1252");
      }
      else if (!JAG_STRICMP("FontSpecific", enc_scheme))
      {
          // not a public encoding - no action
      }
      else
      {
          return find_encoding_record(enc_scheme);
      }

      return encrec;
  }

  ///
  /// For given record looks up a canonical encoding record.
  ///
  CharEncodingRecord const& find_encoding_record(Char const* encoding)
  {
      if (is_empty(encoding))
      {
          JAG_ASSERT(spirit::classic::find(g_encodings, "builtin"));
          return *spirit::classic::find(g_encodings, "builtin");
      }

      CharEncodingRecord const* enc_rec = 0;
      char const* canonical = get_canonical_converter_name(encoding);
      if (canonical)
          enc_rec = spirit::classic::find(g_encodings, canonical);

      if (!enc_rec)
          throw exception_invalid_value(msg_unrecognized_char_enc(encoding)) << JAGLOC;

      return *enc_rec;
  }


} // anonymous namespace




}} // namespace jag::resources

/** EOF @file */
