// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "contentstream.h"
#include "objfmt.h"
#include "docwriterimpl.h"

#include <core/generic/null_deleter.h>
#include <core/jstd/zlib_stream.h>
#include <core/jstd/file_stream.h>
#include <core/jstd/memory_stream.h>

#include <boost/bind.hpp>
#include <boost/ref.hpp>


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
    , m_top_stream(&m_stream, null_deleter)
{
    if (num_filters)
    {
        m_filters.reserve(num_filters);
        m_filter_ids.reserve(num_filters);
        for(int i=num_filters-1; i>=0; --i)
        {
            boost::shared_ptr<ISeqStreamOutputControl> new_filter;
            switch (filters[i])
            {
            case STREAM_FILTER_FLATE:
                new_filter.reset(new jstd::ZLibStreamOutput(*m_top_stream));
                break;
            default:
                ;
            }

            m_filter_ids.push_back(filters[i]);

            if (new_filter)
            {
                m_top_stream = new_filter;
                m_filters.push_back(std::make_pair<>(new_filter, filters[i]));
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
    // delete filters in reverse order as they are chained
    m_top_stream.reset();
    std::size_t cnt=m_filters.size();
    if (cnt > 0 )
    {
        while(cnt--)
            m_filters[cnt].first.reset();
    }
}


//////////////////////////////////////////////////////////////////////////
ObjFmtBasic& ContentStream::object_writer()
{
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


//////////////////////////////////////////////////////////////////////////
void ContentStream::on_output_definition()
{
    m_state |= OUTPUTTED;

    ObjFmt& writer = IndirectObjectImpl::object_writer();

    for (size_t i=0; i<m_filters.size(); ++i)
        m_filters[i].first->close();


    int stream_length = static_cast<int>(m_stream.tell());

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



}} //namespace jag::pdf
