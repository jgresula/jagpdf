/*
 * Copyright (c) 2005-2009 Jaroslav Gresula
 *
 * Distributed under the MIT license (See accompanying file
 * LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
 *

 */
#include "ctestcommon.h"

jag_Int JAG_CALLSPEC write_noop(void* custom, void const* data, jag_ULong size) {
    JAG_IGNORE_UNUSED(custom);
    JAG_IGNORE_UNUSED(data);
    JAG_IGNORE_UNUSED(size);
    return 0;
}

jag_Int JAG_CALLSPEC close_noop(void* custom) {
    JAG_IGNORE_UNUSED(custom);
    return 0;
}

jag_streamout get_noop_stream()
{
    jag_streamout stream_out;
    stream_out.write = write_noop;
    stream_out.close = close_noop;
    stream_out.custom_data = (void*)0;
    return stream_out;
}


