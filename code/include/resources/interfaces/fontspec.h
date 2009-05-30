// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __FONTSPEC_H_JAG_2232__
#define __FONTSPEC_H_JAG_2232__


#include <interfaces/refcounted.h>


namespace jag
{

class IFontSpec
    : public IRefCounted
{
public:
    virtual void filename(Char const* val) = 0;
    virtual void facename(Char const* val) = 0;
    virtual void encoding(Char const* val) = 0;
    virtual void size(Double val) = 0;
    virtual void bold(Int val) = 0;
    virtual void italic(Int val) = 0;
    virtual void adobe14(Int yes) = 0;

protected:
    ~IFontSpec() {}
};

} //namespace jag


#endif //__FONTSPEC_H_JAG_2232__
