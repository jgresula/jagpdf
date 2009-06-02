// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <resources/typeman/truetype/ttstructs.h>
#include <interfaces/streams.h>
#include <string.h>

using namespace boost::integer;

namespace jag { namespace resources { namespace truetype
{


unsigned char g_table_str[TT_NUM_TABLES][4] =
{
    { 'm', 'a', 'x', 'p' },
    { 'c', 'm', 'a', 'p' },
    { 'l', 'o', 'c', 'a' },
    { 'h', 'e', 'a', 'd' },
    { 'g', 'l', 'y', 'f' },
    { 'n', 'a', 'm', 'e' },
    { 'O', 'S', '/', '2' },
    { 'c', 'v', 't', ' ' },
    { 'f', 'p', 'g', 'm' },
    { 'p', 'r', 'e', 'p' },
    { 'h', 'h', 'e', 'a' },
    { 'h', 'm', 't', 'x' },
    { 'p', 'o', 's', 't' },
};


TableStringToVal g_table_string_to_val[TT_NUM_TABLES] =
{
    { g_table_str[TT_MAXP], TT_MAXP },
    { g_table_str[TT_CMAP], TT_CMAP },
    { g_table_str[TT_LOCA], TT_LOCA },
    { g_table_str[TT_HEAD], TT_HEAD },
    { g_table_str[TT_GLYF], TT_GLYF },
    { g_table_str[TT_NAME], TT_NAME },
    { g_table_str[TT_OS2],  TT_OS2  },
    { g_table_str[TT_CVT],  TT_CVT  },
    { g_table_str[TT_FPGM], TT_FPGM },
    { g_table_str[TT_PREP], TT_PREP },
    { g_table_str[TT_HHEA], TT_HHEA },
    { g_table_str[TT_HMTX], TT_HMTX },
    { g_table_str[TT_POST], TT_POST },
};


//////////////////////////////////////////////////////////////////////////
boost::integer::ubig32_t file_checksum(IStreamInput& instream, size_t skip_offset)
{
    const size_t buffer_len = 4096;
    Byte buffer[buffer_len];
    ULong nr_read, read_total=0;
    ubig32_t sum=0;
    bool done = false;
    instream.seek(0, OFFSET_FROM_BEGINNING);
    do {
        done = !instream.read(buffer, buffer_len, &nr_read);
        if (skip_offset<read_total || skip_offset>=read_total+nr_read)
        {
            sum = checksum(buffer, nr_read, sum);
        }
        else
        {
            const size_t skip_offset_local = skip_offset-read_total;
            if (skip_offset_local)
                sum = checksum(buffer, skip_offset_local, sum);

            if (skip_offset_local+4 < nr_read)
                sum = checksum(buffer+skip_offset_local+4, nr_read-skip_offset_local-4, sum);
        }
        read_total += nr_read;
    }
    while(!done);

    return sum;
}


//////////////////////////////////////////////////////////////////////////
ubig32_t checksum(void const* data, size_t len, unsigned int seed)
{
    ubig32_t sum(seed);
    ubig32_t const* curr = static_cast<ubig32_t const*>(data);
    ubig32_t const*const end = curr + len/sizeof(ubig32_t);

    while(curr < end)
        sum += *curr++;

    size_t modulo = len%sizeof(ubig32_t);
    if (modulo)
    {
        ubig32_t last4(0);
        memcpy(&last4, static_cast<Byte const*>(data)+len-modulo, modulo);
        sum += last4;
    }

    return sum;
}


}}} // namespace jag::resources::truetype
