// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __FONTDESCRIPTOR_H_JAG_1953__
#define __FONTDESCRIPTOR_H_JAG_1953__

#include "indirectobjectimpl.h"
#include "indirectobjectref.h"
#include "pdffontdata.h"
#include <set>

namespace jag { namespace pdf
{
class DocWriterImpl;

class FontDescriptor
    : public IndirectObjectImpl
{
public:
    DEFINE_VISITABLE;
    FontDescriptor(DocWriterImpl& doc, PDFFontData const& font_data);

public:
    char const* basename() const;
    bool is_subset() const;
    bool is_embedded() const;
    void add_used_glyphs(UsedGlyphs const& used_glyphs);
    void force_embedding();

public: // IndirectObjectImpl
    void on_output_definition();
    bool on_before_output_definition();

private:
    unsigned int face_flags();

private:
    PDFFontData  m_font_data;
    mutable std::string  m_basename;

    /// Reference to a TrueType font file if it is being embeded.
    IndirectObjectRef  m_font_file2;
    IndirectObjectRef  m_font_file3;

    bool m_force_embedding;

    // set only if the typeface is gonna be subset.
    UsedGlyphs m_used_glyphs;


    /**
     * @brief Set when a method depending on m_force_embedding is invoked.
     * @see force_embedding()
     */
    mutable bool m_method_depending_on_force_embedding_flag_invoked;
    mutable bool m_embedded_regardless_of_copyright;
};


}} //namespace jag::pdf

#endif //__FONTDESCRIPTOR_H_JAG_1953__
