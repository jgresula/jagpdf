// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "pdffile_trailer.h"
#include "docwriterimpl.h"
#include "generic_dictionary.h"
#include "objfmt.h"

#include <jagpdf/detail/version.h>
#include <core/jstd/crt.h>
#include <core/generic/stringutils.h>
#include <interfaces/configinternal.h>
#include <interfaces/execcontext.h>


namespace jag { namespace pdf
{


PDFFileTrailer::PDFFileTrailer(DocWriterImpl& doc)
    : m_doc(doc)
{
}

void PDFFileTrailer::output(
      UInt xref_offset
    , UInt xref_size
    , IIndirectObject const& root
    , IIndirectObject* encrypt)
{
    ObjFmt& writer = m_doc.object_writer();
    writer.raw_text("trailer\n")
        .dict_start()
        .dict_key("Size").space().output(xref_size)
        .dict_key("Root").space().ref(root);

    if (encrypt)
        writer.dict_key("Encrypt").space().ref(*encrypt);

    if (m_info.get())
        writer.dict_key("Info").space().ref(*m_info);

    DocWriterImpl::FileID const& file_id = m_doc.file_id();
    writer
        .dict_key("ID")
        .array_start()
        .unenc_text_string_hex(static_cast<Char const*>(static_cast<void const*>(&file_id[0])), 16)
        .space()
        .unenc_text_string_hex(static_cast<Char const*>(static_cast<void const*>(&file_id[0])), 16)
        .array_end()
    ;


    writer.dict_end()
        .raw_text("\nstartxref\n")
        .output(xref_offset)
        .raw_text("\n%%EOF\n")
    ;
}

void PDFFileTrailer::before_output()
{
    IProfileInternal const& cfg = m_doc.exec_context().config();
    Char const* val;

#    define READ_INFO_DICT_CFG(opt, field)                      \
    val = cfg.get(opt);                           \
    if (!is_empty(val))                                        \
        info_dictionary().insert_string(field, val);


    // The Producer field must have a constant length (42) in order to be able
    // to compare pdf in the test suite
    if (cfg.get_int("info.static_producer"))
    {
        // tests focused on enconding needs a static field
        // make the string always as long as this:  "                                           "
        info_dictionary().insert_string("Producer", "JagPDF library, http://jagpdf.org          ");
    }
    else
    {
        const int buffer_len = 44;
        char buffer_id[buffer_len];
        memset(buffer_id, ' ', buffer_len);
        int written = jstd::snprintf(buffer_id,
                                     buffer_len - 1,
                                     "JagPDF %d.%d.%d, http://jagpdf.org",
                                     this_version_major,
                                     this_version_minor,
                                     this_version_patch);
        JAG_ASSERT(written < buffer_len);
        buffer_id[written] = ' ';
        buffer_id[buffer_len-1] = 0;
        info_dictionary().insert_string("Producer", buffer_id);
    }

    READ_INFO_DICT_CFG("info.title", "Title");
    READ_INFO_DICT_CFG("info.author", "Author");
    READ_INFO_DICT_CFG("info.subject", "Subject");
    READ_INFO_DICT_CFG("info.keywords", "Keywords");
    READ_INFO_DICT_CFG("info.creator", "Creator");

    int creation_date = cfg.get_int("info.creation_date");
    if (!creation_date)
    {
        info_dictionary().remove_key("CreationDate");
    }
    else
    {
        info_dictionary().insert_string("CreationDate", DateWriter().date());
    }

    if (m_info.get())
        m_info->output_definition();

#undef READ_INFO_DICT_CFG
#undef READ_INFO_DICT_CFG_CHKVER
}

GenericDictionary& PDFFileTrailer::info_dictionary()
{
    if (!m_info.get())
        m_info.reset(new GenericDictionary(m_doc));

    return *m_info;
}

}} //namespace jag::pdf
