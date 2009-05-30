// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//

#ifndef __FONTMANAGEMENT_H_JG_1140__
#define __FONTMANAGEMENT_H_JG_1140__

#include "pdffont.h"
#include "pdffontdata.h"
#include "fontdictionary.h"
#include <core/generic/mapsmartptr.h>
#include <resources/utils/resourcetable.h>
#include <core/generic/noncopyable.h>
#include <boost/shared_ptr.hpp>
#include <boost/functional/hash.hpp>
#include <boost/scoped_array.hpp>
#include <boost/ptr_container/ptr_set.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <map>
#include <set>


namespace jag {
namespace pdf {

// fwd
class DocWriterImpl;
class IndirectObjectRef;
class FontDictionary;

// ensures stable sort for FontDictionary objects, otherwise different runs of
// the same program could produce different pdfs
//
struct stable_font_dict_less
{
    bool operator()(FontDictionary const* lhs, FontDictionary const* rhs) const
    {
        return lhs->id() < rhs->id();
    }
};


//
//
//
class FontManagement
    : public noncopyable
{
public:
    FontManagement(DocWriterImpl& doc);

    PDFFont const& font_load(Char const* fspec);
    PDFFont const& font_load(IFontEx const& fnt);

    IndirectObjectRef font_ref(FontDictionary const& font_dict);
    void output_fonts();
    IndirectObjectRef enc_diff_dict(EnumCharacterEncoding enc);

private:
    PDFFont const& lookup_font(std::auto_ptr<PDFFont>& pdffont);

public: //diagnostics
    size_t dbg_num_fonts() const;
    size_t dbg_num_dicts() const;

private:
    // font dictionary types
    typedef boost::ptr_map<PDFFontDictData, FontDictionary> FontDictMap;

    // pdf font types
    typedef boost::ptr_set<PDFFont> FontMap;
    typedef std::set<FontDictionary*,stable_font_dict_less> FontsToOutput;


private:
    DocWriterImpl&  m_doc;
    FontDictMap m_fontdicts;            ///< Keeps font dictionaries.
    FontMap m_fonts;                    ///< Keeps pdf fonts.
    FontsToOutput m_fonts_to_output;    ///< Fonts being referenced and thus gonna be output
    boost::scoped_array<IndirectObjectRef> m_enc_diffs;
    int m_fdict_id;
};




}} //namespace jag::pdf


#endif //__FONTMANAGEMENT_H_JG_1140__






















