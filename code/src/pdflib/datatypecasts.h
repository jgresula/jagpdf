// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef DATATYPECASTS_JG2116_H__
#define DATATYPECASTS_JG2116_H__

#include <interfaces/constants.h>
#include <resources/interfaces/charencoding.h>
#include <utility>


namespace jag {
//fwd
class ITypeface;

namespace pdf {

char const* font_type_str(ITypeface const& face);
char const* encoding_type_str(EnumCharacterEncoding enc);
char const* cmap_str(EnumCharacterEncoding enc);
std::pair<char const*, int> character_collection_str(EnumCharacterEncoding enc, Int version);


}} // namespace jag::pdf

#endif // __DATATYPECASTS_JG2116_H__
/** EOF @file */
