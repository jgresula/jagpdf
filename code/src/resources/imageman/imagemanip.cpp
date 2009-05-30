// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "imagemanip.h"
#include <core/generic/internal_types.h>
#include <interfaces/streams.h>
#include <boost/integer/endian.hpp>

using namespace boost::integer;

namespace jag {
namespace resources {

void downsample_big16_to_8(ISeqStreamInput& in, ISeqStreamOutput& out)
{
    const int buffer_size = 512;
    const int buffer_byte_size = sizeof(ubig16_t)*512;
    ubig16_t in_buffer[buffer_size];
    Byte out_buffer[buffer_size];

    ULong read = 0;
    bool still_has_data;
    do
    {
        still_has_data = in.read(&in_buffer, buffer_byte_size, &read);
        const ULong num_samples = read/sizeof(ubig16_t);
        if (num_samples)
        {
            for (ULong i=0; i<num_samples; ++i)
                out_buffer[i] = static_cast<Byte>(static_cast<UInt16>(in_buffer[i])>>8);

            out.write(out_buffer, num_samples);
        }
    }
    while(still_has_data);
}

}} // namespace jag::resources

/** EOF @file */
