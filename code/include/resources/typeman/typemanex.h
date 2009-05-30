// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __TYPEMANEX_H_JAG_1325__
#define __TYPEMANEX_H_JAG_1325__

#include <resources/interfaces/typeman.h>
#include <resources/interfaces/fontspec.h>
#include <boost/intrusive_ptr.hpp>

namespace jag { namespace resources
{

class ITypeManEx
    : public ITypeMan
{
public:
    virtual ULong dbg_num_typefaces() const = 0;
    virtual void dbg_dump_typefaces() const = 0;
    virtual ULong dbg_num_fonts() const = 0;

protected:
    ~ITypeManEx() {}
};


}} // namespace jag::resources

#endif //__TYPEMANEX_H_JAG_1325__
