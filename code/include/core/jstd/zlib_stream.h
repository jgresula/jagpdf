// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __ZLIB_STREAM_H__272921
#define __ZLIB_STREAM_H__272921

#include <interfaces/streams.h>
#include <core/generic/noncopyable.h>
#include <boost/shared_ptr.hpp>
#include <zlib.h>
#include <memory>

namespace jag { namespace jstd
{

/// zlib implementation of ISeqStreamOutput
class ZLibStreamOutput
    : public ISeqStreamOutputControl
{
public:
    ZLibStreamOutput(ISeqStreamOutput& stream, int compression_level = Z_DEFAULT_COMPRESSION);
    ~ZLibStreamOutput();

public:  //ISeqStreamOutput
    void write(void const* data, ULong size);
    ULong tell() const;
    void flush();

public: //ISeqStreamOutputControl
    void close();

private:
    int DeflateBuffer(int flush_type);

private:
    enum                { CHUNK_SIZE = 16384 };
    ISeqStreamOutput&   m_stream;
    ULong               m_position;
    z_stream            m_strm;
    bool                m_closed;
};


class ZLibStreamInput
    : public ISeqStreamInput
{
public:
    ZLibStreamInput(ISeqStreamInput& stream);
    ZLibStreamInput(std::auto_ptr<ISeqStreamInput> stream);
    ~ZLibStreamInput();

public: //ISeqStreamInput
    bool read(void* data, ULong size, ULong* read);
    ULong tell() const;

public:
    void close();

private:
    void init();

private:
    enum                                { CHUNK_SIZE = 16384 };
    char                                m_in[CHUNK_SIZE];
    boost::shared_ptr<ISeqStreamInput>  m_stream;
    z_stream                            m_strm;
    ULong                               m_position;
    bool                                m_closed;
};

}} //namespace jag::jstd

#endif //__ZLIB_STREAM_H__272921
