// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <resources/typeman/fontutils.h>
#include <resources/interfaces/font.h>
#include <resources/interfaces/typefaceops.h>
#include <boost/functional/hash.hpp>
#include <string.h>

namespace jag {


//////////////////////////////////////////////////////////////////////////
bool operator<(ITypeface const& lhs, ITypeface const& rhs)
{
    return memcmp(lhs.hash(), rhs.hash(), sizeof(Hash16)) < 0;
}


//////////////////////////////////////////////////////////////////////////
bool operator==(ITypeface const& lhs, ITypeface const& rhs)
{
    return !memcmp(lhs.hash(), rhs.hash(), sizeof(Hash16));
}


//////////////////////////////////////////////////////////////////////////
size_t hash_value(ITypeface const& font)
{
    return boost::hash_value(font.hash());
}

namespace resources {


//////////////////////////////////////////////////////////////////////////
bool synthesized_bold(IFontEx const& font)
{
    return font.is_bold() && !font.typeface().bold();
}

//////////////////////////////////////////////////////////////////////////
bool synthesized_italic(IFontEx const& font)
{
    return font.is_italic() && !font.typeface().italic();
}



}} //namespace jag::resources
