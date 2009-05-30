// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __NONCOPYABLE_H_JG1104__
#define __NONCOPYABLE_H_JG1104__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#ifdef _MSC_VER
# pragma warning(disable: 4511)
# pragma warning(disable: 4512)
#endif

namespace jag
{

class noncopyable
{
protected:
    noncopyable() {}
    ~noncopyable() {}
private:
    noncopyable(const noncopyable&);
    const noncopyable& operator=(const noncopyable&);
};

} //namespace jag

#endif

