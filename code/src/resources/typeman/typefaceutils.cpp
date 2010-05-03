// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <resources/typeman/typefaceutils.h>
#include <core/jstd/file_stream.h>
#include <interfaces/streams.h>
#include <core/errlib/errlib.h>

namespace jag {
namespace resources {

//////////////////////////////////////////////////////////////////////////
// FTStreamAdapterFromStream
//////////////////////////////////////////////////////////////////////////

class FTStreamAdapterFromStream
    : public IStreamInput
{
    FT_Stream       m_stream;
    unsigned long   m_current;
public:
    FTStreamAdapterFromStream(FT_Stream stream)
        : m_stream(stream)
        , m_current(0)
    {}

public: //IStreamInput
    bool read(void* data, ULong size, ULong* nr_read) {
        unsigned long read_bytes = (m_stream->read)(m_stream,
                                                     m_current,
                                                     static_cast<unsigned char*>(data),
                                                     static_cast<unsigned long>(size));
        if (nr_read)
            *nr_read = read_bytes;

        m_current += read_bytes;
        return m_current != m_stream->size;
    }

    void seek(Int offset, StreamOffsetOrigin origin)
    {
        switch(origin)
        {
        case OFFSET_FROM_BEGINNING:
            m_current = offset;
            break;

        case OFFSET_FROM_CURRENT:
            m_current += offset;
            break;

        case OFFSET_FROM_END:
            m_current = m_current+offset;
            break;
        }

        if (m_current>m_stream->size)
            m_current=m_stream->size ? m_stream->size-1 : 0;
    }

    ULong tell() const { return m_current; }
};



//////////////////////////////////////////////////////////////////////////
std::auto_ptr<IStreamInput>
create_ftopenargs_stream_adapter(FT_Open_Args const& args)
{
    switch(args.flags)
    {
    case FT_OPEN_STREAM:
        return std::auto_ptr<IStreamInput>(
            new FTStreamAdapterFromStream(args.stream)
       );

    case FT_OPEN_PATHNAME:
        return std::auto_ptr<IStreamInput>(
            new jstd::FileStreamInput(args.pathname)
       );

    default:
        JAG_TBD;
    }

    JAG_INTERNAL_ERROR;
}



}} // namespace jag::resources


