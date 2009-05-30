// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __PDFTYPEFACE_H_JAG_0106__
#define __PDFTYPEFACE_H_JAG_0106__

#include "charencoding.h"
#include <resources/interfaces/resourcehandle.h>
#include <boost/shared_ptr.hpp>
#include <string>

namespace jag
{
class IStreamInput;
class ITypefaceEx;

namespace pdf
{
class DocWriterImpl;

/// PDF specific wrap of ITypefaceEx
class PDFTypeface
{
    DocWriterImpl&    m_doc;
    TypefaceHandle    m_handle;
    CharacterEncoding const& m_encoding;
    std::string        m_basename;
    ITypefaceEx&    m_typeface;
    char            m_subset_prefix[7];

public:
    PDFTypeface(DocWriterImpl& doc, TypefaceHandle handle, CharacterEncoding const& encoding);

    /// value of BaseFont of FontDescriptor
    char const* base_font_name() const;

    /// indicates whether the font program is embedded
    bool is_embedded();

    /// encoding/code page associated with this typeface
    CharacterEncoding const& encoding() const { return m_encoding; }

    /// @pre typeface must be embedded
    boost::shared_ptr<IStreamInput> font_program_to_embed();

    ITypefaceEx& typeface() { return m_typeface;    }

private:
    bool is_subset();
    void generate_subset_prefix();
    void form_base_name();
};

}} //namespace jag::pdf

#endif __PDFTYPEFACE_H_JAG_0106__
