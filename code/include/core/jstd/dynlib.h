// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef DYNLIB_JG1735_H__
#define DYNLIB_JG1735_H__

#include <boost/config.hpp>
#include <boost/noncopyable.hpp>
//#include <core/union_cast.h>

#ifdef _WIN32
# include <windows.h>
#endif


namespace jag {
namespace jstd {

class DynamicLibrary
    : public boost::noncopyable
{
public:
    explicit DynamicLibrary(char const* name);
    ~DynamicLibrary();
    void* symbol(char const*) const;

private:
#   ifdef _WIN32
    HMODULE  m_hlib;
#   else
    void*  m_hlib;
#   endif
};


template<class SYMBOL>
SYMBOL get_dll_symbol(DynamicLibrary const& lib, char const* sym)
{
    // see http://www.trilithium.com/johan/2004/12/problem-with-dlsym/
//    return union_cast<SYMBOL,Strs::DynamicLibrary::DlSymbol>(lib.symbol(sym));

    return (SYMBOL)(lib.symbol(sym));
}



}} // namespace jag::jstd


#endif // DYNLIB_JG1735_H__
/** EOF @file */
