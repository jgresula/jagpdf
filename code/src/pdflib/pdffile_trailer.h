// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __PDFFILE_TRAILER_H__
#define __PDFFILE_TRAILER_H__

#include "generic_dictionary.h"
#include <interfaces/stdtypes.h>
#include <core/generic/noncopyable.h>

#include <boost/scoped_ptr.hpp>

namespace jag { namespace pdf
{
class DocWriterImpl;
class IIndirectObject;

class PDFFileTrailer
    : public noncopyable
{
public:
    explicit PDFFileTrailer(DocWriterImpl& doc);
    void output(UInt xref_offset, UInt xref_size, IIndirectObject const& root, IIndirectObject* encrypt);
    void before_output();

private:
    GenericDictionary& info_dictionary();

private:
    boost::scoped_ptr<GenericDictionary>  m_info;
    DocWriterImpl&              m_doc;
};

}} //namespace jag::pdf

#endif //__PDF_FILE_TRAILER_H__
