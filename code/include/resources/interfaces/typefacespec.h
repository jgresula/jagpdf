// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __TYPEFACESPEC_H_JAG_2108__
#define __TYPEFACESPEC_H_JAG_2108__

#include <interfaces/refcounted.h>


namespace jag
{

class ITypefaceSpec
    : public IRefCounted
{
public:
    virtual void encoding(Char const* enc) = 0;
    virtual void file_name(Char const* file_name) = 0;
    virtual void data(Byte const* array, UInt length) = 0;

protected:
    ~ITypefaceSpec() {}
};

} //namespace jag


#endif //__TYPEFACESPEC_H_JAG_2108__
