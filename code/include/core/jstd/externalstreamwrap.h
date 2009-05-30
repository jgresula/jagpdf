// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef EXTERNALSTREAMWRAP_JG1712_H__
#define EXTERNALSTREAMWRAP_JG1712_H__

#include <interfaces/streams.h>
#include <pdflib/apistructures.h>
#include <jagpdf/detail/c_prologue.h>
#include <core/errlib/errlib.h>
#include <boost/scoped_array.hpp>
#include <core/generic/checked_cast.h>

namespace jag {
namespace jstd {

//
//
//
class ExternalStreamOut
    : public ISeqStreamOutputControl
{
    jag_streamout const*const m_stream;
    ULong                     m_pos;

public:
    ExternalStreamOut(jag_streamout const* stream)
        : m_stream(stream)
        , m_pos(0)
    {}

public:
    void write(void const* data, ULong size) {
        Int err=m_stream->write(m_stream->custom_data, data, size);
        if (err)
        {
            throw exception_io_error(msg_cannot_write_stream())
                << user_errno_info(err)
                << io_object_info("external stream")
                << JAGLOC;
        }

        m_pos += size;
    }


    void close() {
        Int err=m_stream->close(m_stream->custom_data);
        if (err)
        {
            throw exception_io_error(msg_cannot_close_stream())
                << user_errno_info(err)
                << io_object_info("external stream")
                << JAGLOC;
        }
    }

    ULong tell() const {
        return m_pos;
    }

    void flush() { /*no op*/ }
};

//
//
//
class ExternalSWIGStreamOut
    : public ISeqStreamOutputControl
{
    boost::intrusive_ptr<apiinternal::StreamOut> m_stream;
    ULong                                        m_pos;
    boost::scoped_array<Byte> m_buffer;
    enum {BUFFER_SIZE = 4096};
    Byte* m_buffptr;
    Byte* const m_buffend;

public:
    ExternalSWIGStreamOut(apiinternal::StreamOut* stream)
        : m_stream(stream)
        , m_pos(0)
        , m_buffer(new Byte[BUFFER_SIZE])
        , m_buffptr(&m_buffer[0])
        , m_buffend(m_buffptr + BUFFER_SIZE)
    {}

public:
    void write(void const* data, ULong size)
    {
        m_pos += size;
        Byte const* const data_in = jag_reinterpret_cast<Byte const*>(data);

        if (size > BUFFER_SIZE)
        {
            // if the input data size is greater than size of the buffer then
            // flush the buffer and write whole input at once
            flush();
            m_stream->write(data, size);
            m_stream->check_error();
        }
        else
        {
            ULong buff_size = m_buffend - m_buffptr;
            ULong to_be_copied = (std::min)(size, buff_size);
            memcpy(m_buffptr, data, to_be_copied);
            size -= to_be_copied;
            m_buffptr += to_be_copied;

            if (size)
            {
                // still some data to process
                JAG_ASSERT(size < BUFFER_SIZE);
                JAG_ASSERT(m_buffptr == m_buffend);
                flush(); // buffer is full
                memcpy(&m_buffer[0], data_in + to_be_copied, size);
                m_buffptr += size;
            }
        }
    }

    void flush()
    {
        ULong size = m_buffptr - &m_buffer[0];
        if (size)
        {
            m_stream->write(&m_buffer[0], size);
            m_stream->check_error();
            m_buffptr = &m_buffer[0];
        }
    }


    void close() {
        flush();
        m_stream->close();
        m_stream->check_error();
    }

    ULong tell() const {
        return m_pos;
    }

};



}} // namespace jag::jstd

#endif // EXTERNALSTREAMWRAP_JG1712_H__
/** EOF @file */
