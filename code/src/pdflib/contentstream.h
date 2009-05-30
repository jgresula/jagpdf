// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//



#ifndef __CONTENTSTREAM_H__1632120
#define __CONTENTSTREAM_H__1632120

#include "indirectobjectimpl.h"
#include "generic_dictionary_impl.h"
#include "objfmt.h"
#include "defines.h"

#include <interfaces/streams.h>
#include <core/jstd/memory_stream.h>
#include <boost/shared_ptr.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/function.hpp>
#include <vector>

namespace jag {
namespace pdf {
class DocWriterImpl;

/// pdf content stream implementation
class ContentStream
    : public IndirectObjectImpl
{
public:
    DEFINE_VISITABLE

    ContentStream(DocWriterImpl& doc, StreamFilter const* filters=0, int num_filters=0);
    ~ContentStream();
    ISeqStreamOutput& stream();
    ObjFmtBasic& object_writer();
    bool is_empty() const;

    /// allows to add data to stream dictionary
    typedef boost::function<void (ObjFmt& fmt)> callback_t;
    void set_writer_callback(callback_t const& writer);

private: // IndirectObjectImpl
    void on_output_definition();
    bool on_before_output_definition();

private:
    static char const*const s_filter_names[];
    typedef std::pair<boost::shared_ptr<ISeqStreamOutputControl>, StreamFilter> FilterEntry;

    enum State
    {
        INITIAL =          0,
        OUTPUTTED =        1U << 0,
        NON_EMPTY_STREAM = 1U << 1,
    };

    jstd::MemoryStreamOutput          m_stream;
    unsigned int                         m_state;
    std::vector<FilterEntry>             m_filters;
    boost::shared_ptr<ISeqStreamOutput>  m_top_stream;
    std::vector<StreamFilter>            m_filter_ids;
    boost::shared_ptr<ObjFmtBasic>       m_object_writer;
    callback_t                           m_writer_callback;
};

}} //namespace jag::pdf

#endif //__CONTENTSTREAM_H__1632120
