// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __PDFFONT_H_JAG_1105__
#define __PDFFONT_H_JAG_1105__

#include <core/jstd/uconverterctrl.h>
#include <core/jstd/unicode.h>
#include <core/generic/noncopyable.h>
#include <core/generic/assert.h>
#include <resources/interfaces/font.h>
#include <boost/shared_ptr.hpp>
#include <boost/intrusive_ptr.hpp>
#include <map>


namespace jag {
namespace pdf {

// fwd
class FontDictionary;
class PDFFont;
class FontManagement;

//
//
//
class PDFFontIterator
{
public:
    PDFFontIterator(PDFFont const& obj, Char const* begin, Char const* end);
    PDFFont const* next(std::vector<Char>& str);

private:
    PDFFont const* m_obj;
    jstd::UnicodeToCPIterator m_iter;
};

//
//
//
class PDFFont
    : public noncopyable
{
public:
    PDFFont(FontManagement& font_mgm,
            IFontEx const* font,
            FontDictionary* font_dict);

    IFontEx const* font() const { return m_font; }
    FontDictionary& font_dict() const {
        JAG_PRECONDITION(!has_multiple_encondings());
        return *m_font_dict;
    }

    jag::jstd::UnicodeConverter* acquire_converter() const;
    void release_converter() const;

    bool has_multiple_encondings() const;
    PDFFontIterator create_iterator(Char const* begin,
                                    Char const* end) const;
    friend class PDFFontIterator;
    friend bool operator<(PDFFont const& lhs, PDFFont const& rhs);

private:
    IFontEx const* m_font;
    FontDictionary* m_font_dict;
    FontManagement& m_font_mgm;
    jstd::UConverterCtrl m_conv_ctrl;
    typedef std::map<EnumCharacterEncoding, PDFFont const*> EncMap;
    mutable EncMap m_enc_to_font;
};



bool operator<(PDFFont const& lhs, PDFFont const& rhs);

}} //namespace jag::pdf

#endif //__PDFFONT_H_JAG_1105__
