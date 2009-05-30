// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __GENERICCONTENTSTREAM_H_JAG_0010__
#define __GENERICCONTENTSTREAM_H_JAG_0010__

#include "contentstream.h"
#include "indirectobjectfwd.h"
#include "docwriterimpl.h"
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>

namespace jag { namespace pdf
{

class GenericContentStream
    : public IndirectObjectFwd
{
public:
    DEFINE_VISITABLE;
    typedef boost::function<void (ObjFmt&)> callback_t;

    GenericContentStream(DocWriterImpl& doc, callback_t const& t)
        : m_content_stream(doc.create_content_stream())
    {
        m_content_stream->set_writer_callback(t);
        reset_indirect_object_worker(m_content_stream.get());
    }

public:
    ISeqStreamOutput& out_stream() const {
        return m_content_stream->stream();
    }

private:
    boost::scoped_ptr<ContentStream> m_content_stream;
};

}} //namespace jag::pdf

#endif //__GENERICCONTENTSTREAM_H_JAG_0010__
