// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __TYPEMANIMPL_H_JAG_0020__
#define __TYPEMANIMPL_H_JAG_0020__

#include <resources/typeman/typemanex.h>
#include <resources/interfaces/font.h>
#include <resources/interfaces/typefaceops.h>
#include <resources/utils/resourcetable.h>
#include <resources/typeman/fontimpl.h>

#include <boost/intrusive_ptr.hpp>
#include <boost/ref.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/ptr_container/ptr_set.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <set>

namespace jag {
class IExecContext;

namespace resources {
struct CharEncodingRecord;

///
/// Manages typefaces and fonts.
///
/// One typeface can be shared among multiple fonts. All incoming font
/// requests are matched with already registered ones so there should
/// not be duplicate typefaces or fonts.
///
/// There are two encondings: text encoding and typeface encoding. The
/// typeface encoding is an encoding that is defined by the typeface
/// creator and can be retrieved by invoking
/// ITypeface::encoding_scheme(). The text encoding is the one
/// defined by the client in the font request. This is the encoding
/// the client will use when writing a text with this
/// font. Internally, it can be also used for matching appropriate
/// typeface (e.g. windows system font matcher).
///
/// Clients works with a concept of font. The typeface concept is
/// hidden from them.
///
/// The font request can come either in form of IFontSpec object or as
/// a string.
///
///
class TypeManImpl
    : public ITypeManEx
{
public: // ITypeMan
    boost::intrusive_ptr<IFontSpec> define_font() const;

    IFontEx const& font_load_spec(
        boost::intrusive_ptr<IFontSpec> const& font,
        IExecContext const& exec_ctx);

    IFontEx const& font_load(char const* spec, IExecContext const& exec_ctx);

public: // ITTypeManEx
    ULong dbg_num_typefaces() const;
    ULong dbg_num_fonts() const;
    void dbg_dump_typefaces() const;

public:
    TypeManImpl();

private:
    void check_synthesized_font(
        FontSpecImpl const& fspec,
        ITypeface& typeface,
        IExecContext const& exec_ctx) const;

    std::auto_ptr<ITypeface> create_typeface_from_file(
        FontSpecImpl const& spec,
        IExecContext const& ctx,
        CharEncodingRecord const& enc_rec) const;

    std::auto_ptr<ITypeface> create_typeface_by_search(
        FontSpecImpl const& spec,
        IExecContext const& ctx,
        CharEncodingRecord const& enc_rec) const;

    std::auto_ptr<ITypeface> create_typeface_adobe14(
        FontSpecImpl const& spec) const;

    IFontEx const& font_load_main(
        boost::intrusive_ptr<IFontSpec> spec,
        IExecContext const& exec_ctx);

    IFontEx const& font_load_internal(
        FontSpecImpl const& fspec,
        IExecContext const& exec_ctx);

    IFontEx const& create_adobe14_multienc_font(
        FontSpecImpl const& spec,
        IExecContext const& ctx);

    template<class FONT>
    IFontEx const& lookup_font(std::auto_ptr<FONT>& font,
                               std::set<boost::reference_wrapper<FONT> >& map);


private:
    typedef boost::ptr_set<ITypeface> Typefaces;
    Typefaces m_typefaces;

    typedef boost::reference_wrapper<FontImpl> FontImplRef;
    typedef std::set<FontImplRef> FontsMap;
    FontsMap m_fonts_map;

    typedef boost::reference_wrapper<MultiEncFontImpl> MultiEncFontImplRef;
    typedef std::set<MultiEncFontImplRef> MultiEncFontsMap;
    MultiEncFontsMap m_multi_enc_fonts_map;


    typedef boost::ptr_vector<IFontEx> FontsStorage;
    FontsStorage m_fonts_storage;

    boost::shared_ptr<FT_LibraryRec_> m_ft_library;
};


}} // namespace jag::resources

#endif //__TYPEMANIMPL_H_JAG_0020__

/** EOF @file */
