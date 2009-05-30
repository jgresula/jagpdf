// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <interfaces/streams.h>
#include <core/jstd/memory_stream.h>
#include <core/jstd/streamhelpers.h>

namespace jag {    namespace jstd
{


//////////////////////////////////////////////////////////////////////////
std::string GetStringFromMemoryStream(MemoryStreamOutput const& mem_stream)
{
    return std::string(
        reinterpret_cast<const char*>(mem_stream.data()),
        0,
        static_cast<unsigned int>(mem_stream.tell()));
}

//////////////////////////////////////////////////////////////////////////
void WriteStringToSeqStream(ISeqStreamOutput& seq_stream, char const* str)
{
    seq_stream.write(str, static_cast<UInt>(std::char_traits<char>::length(str)));
}

//////////////////////////////////////////////////////////////////////////
void copy_stream(ISeqStreamInput& src, ISeqStreamOutput& dest)
{
    const int buffer_size = 4096;
    Byte buffer[buffer_size];

    ULong nr_read=0;
    while(src.read(buffer, buffer_size, &nr_read))
    {
        dest.write(buffer, nr_read);
    }
    dest.write(buffer, nr_read);
}


std::auto_ptr<IStreamInput> create_stream_from_memory(void const* mem, size_t memsize)
{
    return std::auto_ptr<IStreamInput>(
        new MemoryStreamInput(mem, memsize));
}

}} //namespace jag::jstd
