// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __FONTUTILS_H_JAG_2042__
#define __FONTUTILS_H_JAG_2042__

namespace jag
{
//fwd
class IFontEx;

namespace resources
{

/// determines whether given font has synthesized bold attribute
bool synthesized_bold(IFontEx const& font);

/// determines whether given font has synthesized italic attribute
bool synthesized_italic(IFontEx const& font);


}} //namespace jag::resources

#endif //__FONTUTILS_H_JAG_2042__
