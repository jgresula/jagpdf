// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __ARCFOUR_STREAM_H__
#define __ARCFOUR_STREAM_H__

#include <interfaces/streams.h>
#include <core/generic/noncopyable.h>

namespace jag { namespace jstd
{

class ArcFourStream
    : public ISeqStreamOutput
{
public:
    ArcFourStream(ISeqStreamOutput& out_stream, Byte const* key, int bit_length);

public: //ISeqStreamOutput
    void write(void const* data, ULong size);
    ULong tell() const;
    void flush();

private:
    Byte        m_s[256];
    int                    m_i;
    int                    m_j;
    ISeqStreamOutput&    m_out_stream;
};

}} //namespace jag::jstd

#endif //__ARCFOUR_STREAM_H__
