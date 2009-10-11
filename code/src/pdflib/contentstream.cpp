// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "contentstream.h"
#include "objfmt.h"
#include "docwriterimpl.h"

#include <core/jstd/zlib_stream.h>
#include <core/jstd/file_stream.h>
#include <core/jstd/memory_stream.h>
#include <core/jstd/streamhelpers.h>
#include <core/errlib/errlib.h>

#include <boost/bind.hpp>
#include <boost/ref.hpp>

using namespace jag::jstd;

namespace jag {
namespace pdf {

char const*const ContentStream::s_filter_names[] = { "FlateDecode", "DCTDecode" };


/**
 * @brief Constructor.
 *
 * @param body pdf file body
 * @param filters an ordered filters specification
 * @param number of filters
 */
ContentStream::ContentStream(DocWriterImpl& doc, StreamFilter const* filters, int num_filters)
    : IndirectObjectImpl(doc)
    , m_state(INITIAL)
    , m_top_stream(&m_stream)
{
    if (num_filters)
    {
        m_filters.reserve(num_filters);
        m_filter_ids.reserve(num_filters);
        for(int i=num_filters-1; i>=0; --i)
        {
            std::auto_ptr<ISeqStreamOutputControl> new_filter;
            switch (filters[i])
            {
            case STREAM_FILTER_FLATE:
                new_filter.reset(new ZLibStreamOutput(*m_top_stream));
                break;
                
            case STREAM_FILTER_DCTDECODE:
                // =JPEG, no actual filter is instantiated; the client sends
                // already encoded data
                break;
                
            default:
                JAG_INTERNAL_ERROR;
            }

            m_filter_ids.push_back(filters[i]);

            if (new_filter.get())
            {
                m_top_stream = new_filter.get();
                m_filters.push_back(new_filter.release());
            }
        }
    }

    m_object_writer.reset(new ObjFmtBasic(*m_top_stream, doc.utf8_to_16be_stream()));
}


//
//
//
ContentStream::~ContentStream()
{
    delete_filters();
}

//
//
//
void ContentStream::delete_filters()
{
    // Delete filters in reverse order as they are chained and redirect
    // m_top_stream to the physical stream.
    //
    // Upon finishing this function:
    // - m_object_writer can't be used as it might be using a dangling pointer
    // - data written to this content stream are not encoded
    //
    m_top_stream = &m_stream;
    std::size_t cnt = m_filters.size();
    if (cnt > 0)
    {
        while(cnt--)
            delete m_filters[cnt];
    }
    m_filters.clear();
}


//////////////////////////////////////////////////////////////////////////
ObjFmtBasic& ContentStream::object_writer()
{
    JAG_PRECONDITION_MSG(m_object_writer, "object already outputted");
    return *m_object_writer;
}

//////////////////////////////////////////////////////////////////////////
ObjFmtBasic const& ContentStream::object_writer() const
{
    // This precondition could be removed since the writer is const. That would
    // involve not resetting m_object_writer in on_output_definition()
    JAG_PRECONDITION_MSG(m_object_writer, "object already outputted");
    return *m_object_writer;
}



//////////////////////////////////////////////////////////////////////////
bool ContentStream::on_before_output_definition()
{
    if (m_stream.tell())
    {
        m_state |= NON_EMPTY_STREAM;
        return true;
    }

    return false;
}

//
//
//
void ContentStream::close_filters()
{
    if (!(m_state & CLOSED_FILTERS))
    {
        for (size_t i=0; i<m_filters.size(); ++i)
            m_filters[i]->close();

        m_state |= CLOSED_FILTERS;
    }
}


//////////////////////////////////////////////////////////////////////////
void ContentStream::on_output_definition()
{
    m_state |= OUTPUTTED;

    close_filters();

    int stream_length = static_cast<int>(m_stream.tell());
    ObjFmt& writer = IndirectObjectImpl::object_writer();
    
    writer.dict_start()
        .dict_key("Length").space().output(stream_length);

    // write filters (if any were used)
    if (m_filter_ids.size())
    {
        writer.dict_key("Filter");

        if (m_filters.size() > 1)
            writer.array_start();

        for(int i= static_cast<int>(m_filter_ids.size())-1; i>=0; --i)
                writer.output(s_filter_names[m_filter_ids[i]]);

        if (m_filters.size() > 1)
            writer.array_end();
    }

    if (m_writer_callback)
        m_writer_callback(writer);

    writer.dict_end()
        .raw_text("stream\n")
        .stream_data(m_stream.data(), stream_length)
        .raw_text("\nendstream")
    ;

    m_stream.close();
    m_object_writer.reset();
}


//////////////////////////////////////////////////////////////////////////
bool ContentStream::is_empty() const
{
    if (m_state&OUTPUTTED)
        return !(m_state&NON_EMPTY_STREAM);

    return m_stream.tell() ? false : true;
}


//////////////////////////////////////////////////////////////////////////
ISeqStreamOutput& ContentStream::stream()
{
    return *m_top_stream;
}



void ContentStream::set_writer_callback(callback_t const& writer)
{
    m_writer_callback = writer;
}


//
// Copies the content stream bytes and the state of the object writer.
//
// This is intended for taking a *read-only* snapshot of the content
// stream. Upon finishing 'this' becomes read-only as well.
//
// It is assumed that the other stream has the same filter stack.
// 
void ContentStream::copy_to(ContentStream& other)
{
    JAG_PRECONDITION(m_filter_ids.size() == other.m_filter_ids.size());
#ifdef JAG_DEBUG
    for(size_t i=0; i<m_filter_ids.size(); ++i)
        JAG_PRECONDITION(m_filter_ids[i] == other.m_filter_ids[i]);
#endif
    
    // Close (i.e. flush) filters so the any pending data (e.g. in zlib) are
    // writen to the physical stream.
    close_filters();

    // Delete the filter stack in the other content stream since we are going to
    // copy raw data.
    other.delete_filters();

    // Copy stream data & object writer state
    MemoryStreamInput in_stream(m_stream.data(), m_stream.tell(), false);
    copy_stream(in_stream, other.stream());
    object_writer().copy_to(other.object_writer());
}



}} //namespace jag::pdf
