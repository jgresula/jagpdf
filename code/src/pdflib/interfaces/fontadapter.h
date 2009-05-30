// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef FONTADAPTER_JG1121_H__
#define FONTADAPTER_JG1121_H__

#include <resources/interfaces/fontinfo.h>

namespace jag {
namespace pdf {

class IFontAdapter
    : public IFont
{
public:
    virtual PDFFont const& font() const = 0;
    virtual ~IFontAdapter() {}
};

}} // namespace jag::pdf

#endif // FONTADAPTER_JG1121_H__
/** EOF @file */
