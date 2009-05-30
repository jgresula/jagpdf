// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __graphicsstatedict_h_JG2210__
#define __graphicsstatedict_h_JG2210__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif


namespace jag
{

/// represent graphics state dictionary
class IGraphicsStateDictionary
{
public:
    virtual void LineWidth(double width) = 0;
    virtual void LineCap(LineCapStyle cap_style) = 0;
};

} //namespace jag


#endif //__graphicsstatedict_h_JG2210__

