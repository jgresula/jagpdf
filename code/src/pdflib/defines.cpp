// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "defines.h"
#include <core/errlib/msg_writer.h>

namespace jag { namespace pdf
{

namespace {
    char const*const rendering_intent_strings[4] =
    {
        "AbsoluteColorimetric",
        "RelativeColorimetric",
        "Saturation",
        "Perceptual"
    };
} // anonymous namespace

//////////////////////////////////////////////////////////////////////////
char const* rendering_intent_string(int index)
{
    if (index<0 || index>3)
    {
        write_message(WRN_IMAGE_RENDERING_INTENT_OUT_OF_RANGE);
        return rendering_intent_strings[1];
    }
    else
    {
        return rendering_intent_strings[index];
    }
}


}} //namespace jag::pdf


