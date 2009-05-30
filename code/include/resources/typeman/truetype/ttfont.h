// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __TTFONT_H_JG_2048__
#define __TTFONT_H_JG_2048__

#include "ttstructs.h"
#include "ttfontparser.h"

#include <map>
#include <boost/shared_ptr.hpp>

namespace jag
{
class IStreamInput;
class ISeqStreamOutput;

namespace resources {
namespace truetype {

// TODO:
//  - new operation - leave only specified tables (PDF_simple_font, PDF_cid)
//  - subsetting
//    - an option whether to include 'cmap'
//    - do not include some tables that are now being included

class TTFont
{
public:
    TTFont(IStreamInput& font_data);
    /**
     * @brief creates a subset of a truetype font
     *
     * @param subset_font output stream the subset font is written to
     * @param subset an array of codepoints (duplicates are not allowed)
     * @param subset_len length of the array of codepoints
     */
    void make_subset(ISeqStreamOutput& subset_font, UInt const subset[], size_t subset_len, bool include_cmap);

    template<class Iterator>
    void make_subset(ISeqStreamOutput& subset_font, Iterator begin, Iterator end, bool include_cmap);

    Char const* postscript_name();

    std::pair<void const*,size_t> dbg_load_glyph(UInt codepoint);

private:
    typedef std::map<UInt,UInt> CodepointToIndex;
    void make_subset_internal(ISeqStreamOutput& subset_font, CodepointToIndex& cp2index, bool include_cmap);

private:
    TTFontParser                    m_ttparser;
    boost::shared_ptr<IStreamInput>    m_font;
};



//////////////////////////////////////////////////////////////////////////
template<class Iterator>
void TTFont::make_subset(ISeqStreamOutput& subset_font, Iterator begin, Iterator end, bool include_cmap)
{
    // build codepoint to glyph index map
    CodepointToIndex codepoint_to_index;
    for(; begin!=end; ++begin)
    {
        UInt glyph_index = m_ttparser.charcode_to_glyph_index(*begin);
        if (glyph_index)
            codepoint_to_index.insert(CodepointToIndex::value_type(*begin, glyph_index));
    }

        make_subset_internal(subset_font, codepoint_to_index, include_cmap);
}


}}} // namespace jag::resources::truetype

#endif //__TFONT_H_JG_2048__
