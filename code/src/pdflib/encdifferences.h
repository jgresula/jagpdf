// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef ENCDIFFERENCES_H_JAG__
#define ENCDIFFERENCES_H_JAG__

#include <interfaces/stdtypes.h>
#include <resources/interfaces/charencoding.h>

namespace jag {
namespace pdf {

// retrieves differences string against WinAnsiEncoding
Char const* enc_differences(EnumCharacterEncoding enc);
bool supports_differences(EnumCharacterEncoding enc);

}} //namespace jag::pdf

#endif //ENCDIFFERENCES_H_JAG__
