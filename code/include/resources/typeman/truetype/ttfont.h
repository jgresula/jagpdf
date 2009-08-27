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

namespace jag {
class IStreamInput;
class ISeqStreamOutput;
class UsedGlyphs;

namespace resources {
namespace truetype {

// TODO:
//  - new operation - leave only specified tables (PDF_simple_font, PDF_cid)
//  - subsetting
//    - do not include some tables that are now being included

class TTFont
{
public:
    TTFont(IStreamInput& font_data);
    /**
     * @brief creates a subset of a truetype font
     *
     * @param subset_font output stream the subset font is written to
     * @param glyphs glyphs to be included
     */
    void make_subset(ISeqStreamOutput& subset_font,
                     UsedGlyphs const& glyphs,
                     bool include_cmap);

    Char const* postscript_name();

    std::pair<void const*,size_t> dbg_load_glyph(UInt codepoint);

private:
    TTFontParser                    m_ttparser;
    boost::shared_ptr<IStreamInput>    m_font;
};



}}} // namespace jag::resources::truetype

#endif //__TFONT_H_JG_2048__
