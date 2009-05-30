// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __CIDFONTDICTIONARY_H_2336__
#define __CIDFONTDICTIONARY_H_2336__

#include "indirectobjectimpl.h"
#include <resources/interfaces/resourcehandle.h>
#include <core/generic/internal_types.h>
#include <vector>


namespace jag {
namespace pdf {

class FontDictionary;

class CIDFontDictionary
    : public IndirectObjectImpl
{
public:
    // needs typeface, used gids, font desc (basename)
    DEFINE_VISITABLE;
    CIDFontDictionary(DocWriterImpl& doc, FontDictionary& fdesc);

protected:
    void on_output_definition();

private:
    void output_widths();
    void fetch_widths(std::vector<UInt16> const& cids, std::vector<Int>& widths);

private:
    FontDictionary const& m_fdict;
};



}} //namespace jag::pdf

#endif //__CIDFONTDICTIONARY_H_2336__
