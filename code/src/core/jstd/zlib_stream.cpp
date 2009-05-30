// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <interfaces/stdtypes.h>
#include <core/jstd/zlib_stream.h>
#include <core/errlib/errlib.h>
#include <core/generic/minmax.h>
#include <core/generic/null_deleter.h>
#include <core/generic/assert.h>
#include <algorithm>

namespace jag {
namespace jstd {

/**
 * @brief initializes zlib output filter
 *
 * @param stream stream to write
 * @param compression_level compression level
 *
 * @exception exception_zlib_deflate_init_failed if zlib initialization failed
 */
ZLibStreamOutput::ZLibStreamOutput(ISeqStreamOutput& stream, int compression_level)
    : m_stream(stream)
    , m_position(0)
    , m_closed(false)
{
    m_strm.zalloc = Z_NULL;
    m_strm.zfree = Z_NULL;
    m_strm.opaque = Z_NULL;
    int ret = deflateInit(&m_strm, compression_level);
    if (ret != Z_OK)
        throw exception_io_error(msg_zlib_deflate_init_failed()) << JAGLOC;
}


/**
 * @brief dtor
 *
 * closes the stream (if still opened)
 */
ZLibStreamOutput::~ZLibStreamOutput()
{
    try
    {
        close();
    }
    catch(std::exception&)
    {
        // can't do much here
    }
}


/**
 * @brief encodes the data
 *
 * @param data data to encode
 * @param size size of data
 */
void ZLibStreamOutput::write(void const* data, ULong size)
{
    if (size)
    {
        m_position += size;
        m_strm.avail_in = static_cast<int>(size);
        m_strm.next_in = reinterpret_cast<Bytef*>(const_cast<void*>(data));
        DeflateBuffer(Z_NO_FLUSH);
    }
}


/**
 *  @return number of written bytes
 */
ULong ZLibStreamOutput::tell() const
{
    return m_position;
}


/**
 *  @brief closes the zlib encoder
 *
 *  @exception exception_zlib_deflate_failed, exception_zlib_deflate_end
 */
void ZLibStreamOutput::close()
{
    if (m_closed)
        return;

    m_strm.avail_in = 0;
    m_strm.next_in = Z_NULL;
    m_closed = true;

    // first, finish the zlib stream and cleanup zlib internal structures
    // error handling is done afterwards, as we need to ensure that deflateEnd()
    // was invoked
    int deflate_ret_val = Z_STREAM_END;
    if (m_strm.total_in)
        deflate_ret_val = DeflateBuffer(Z_FINISH);

    int deflate_end_ret_val = deflateEnd(&m_strm);

    if (Z_STREAM_END != deflate_ret_val)
        throw exception_io_error(msg_zlib_deflate_failed()) << JAGLOC;

    if (Z_OK != deflate_end_ret_val)
        throw exception_io_error(msg_zlib_deflate_end()) << JAGLOC;
}

void ZLibStreamOutput::flush()
{
    m_strm.avail_in = 0;
    m_strm.next_in = Z_NULL;
    DeflateBuffer(Z_SYNC_FLUSH);

    m_stream.flush();
}

int ZLibStreamOutput::DeflateBuffer(int flush_type)
{
    int ret = Z_ERRNO;
    do
    {
        char out[CHUNK_SIZE];
        m_strm.avail_out = CHUNK_SIZE;
        m_strm.next_out = reinterpret_cast<Bytef*>(out);

        ret = deflate(&m_strm, flush_type);
        if (ret < 0)
            throw exception_io_error(msg_zlib_deflate_failed()) << JAGLOC;

        int have = CHUNK_SIZE - m_strm.avail_out;
        if (have)
            m_stream.write(out, have);
    }
    while(m_strm.avail_out == 0);

    return ret;
}



/**
 * @brief ctor, initializes zlib input filter
 *
 * @param stream stream containing deflated data
 */
ZLibStreamInput::ZLibStreamInput(ISeqStreamInput& stream)
    : m_stream(boost::shared_ptr<ISeqStreamInput>(&stream, null_deleter))
    , m_position(0)
    , m_closed(false)
{
    init();
}



ZLibStreamInput::ZLibStreamInput(std::auto_ptr<ISeqStreamInput> stream)
    : m_stream(boost::shared_ptr<ISeqStreamInput>(stream.release()))
    , m_position(0)
    , m_closed(false)
{
    init();
}



void ZLibStreamInput::init()
{
    m_strm.zalloc = Z_NULL;
    m_strm.zfree = Z_NULL;
    m_strm.opaque = Z_NULL;
    m_strm.avail_in = 0;
    m_strm.next_in = Z_NULL;
    int ret = inflateInit(&m_strm);
    if (ret != Z_OK)
        throw exception_io_error(msg_zlib_inflate_init_failed()) << JAGLOC;
}



// dtor, closes the filter (is still opened)
ZLibStreamInput::~ZLibStreamInput()
{
    close();
}

//////////////////////////////////////////////////////////////////////////
bool ZLibStreamInput::read(void* data, ULong size, ULong* read)
{
    // reading chunks > sizeof(unsigned) not supported as zlib accepts
    // 32-bit integers
    JAG_PRECONDITION(size <= 0xffffffffu);

    ULong read_unit = min<ULong>(size, CHUNK_SIZE);
    int ret = Z_OK;

    // set up the zlib output stream, we are going to write right to the passed buffer
    m_strm.avail_out = static_cast<UInt>(size);
    m_strm.next_out = reinterpret_cast<Bytef*>(const_cast<void*>(data));
    try
    {
        do
        {
            // check for a leftover from the previous call
            if (!m_strm.avail_in)
            {
                // fetch data from the input stream
                ULong in_length = read_unit;
                m_stream->read(m_in, in_length, &in_length);
                m_strm.avail_in = static_cast<uInt>(in_length);
                m_strm.next_in = reinterpret_cast<Bytef*>(m_in);
            }

            if (!m_strm.avail_in)
                throw exception_io_error(msg_zlib_inflate_no_more_data_available()) << JAGLOC;

            // inflate the data
            ret = inflate(&m_strm, Z_NO_FLUSH);

            if (ret == Z_STREAM_END)
                break;

            if (ret != Z_OK)
                throw exception_io_error(msg_zlib_inflate_failed()) << JAGLOC;
        }
        while(m_strm.avail_out != 0);

    }
    catch (std::exception& exc)
    {
        m_closed = true;
        inflateEnd(&m_strm);
        throw exc;
    }

    // summarize
    size = size - m_strm.avail_out;
    m_position += size;
    if (read)
        *read = size;

    // check EOF
    if (ret == Z_STREAM_END)
    {
        m_closed = true;
        inflateEnd(&m_strm);
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////
ULong ZLibStreamInput::tell() const
{
    return m_position;
}

//////////////////////////////////////////////////////////////////////////
void ZLibStreamInput::close()
{
    if (m_closed)
        return;

    m_closed = true;
    inflateEnd(&m_strm);
}


//////////////////////////////////////////////////////////////////////////
// unit tests section
//////////////////////////////////////////////////////////////////////////
#ifdef PDF_UNIT_TEST
#include <boost/test/auto_unit_test.hpp>
#include <jstd/memory_stream.h>
#include <string>
#include <algorithm>
#include <vector>

void ZLibTest()
{
    MemoryStreamOutput mo;

    char strs[] = "abcdefghijklmnopqrstuvwxyz123456789";
    int strs_len = static_cast<int>(std::char_traits<char>::length(strs));

    {
        ZLibStreamOutput z(mo);
        for(int i=0; i<3*strs_len; ++i)
        {
            std::rotate(strs, strs+1, strs+strs_len);
            z.write(strs, strs_len);
        }
        z.close();
    }
    mo.close();

    {
        MemoryStreamInput mi(mo.Data(), static_cast<int>(mo.tell()));
        ZLibStreamInput zi(mi);
        std::vector<char> received(strs_len);
        bool ret = false;
        bool inflate_succeeded = true;

        for(int i=0; i<3*strs_len; ++i)
        {
            std::rotate(strs, strs+1, strs+strs_len);
            ret = zi.read(&received[0], strs_len, 0);
            if (0 != memcmp(strs, &received[0], strs_len))
            {
                inflate_succeeded = true;
                break;
            }
        }

        BOOST_CHECK(inflate_succeeded);
        BOOST_CHECK(ret == false);

        zi.close();
        mi.close();
    }
}

#endif //PDF_UNIT_TEST

}} //namespace jag::jstd
