// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __TYPEFACESPECIMPL_H_JAG_2239__
#define __TYPEFACESPECIMPL_H_JAG_2239__

#include <resources/interfaces/typefacespec.h>
#include <core/jstd/memory_stream.h>
#include <core/generic/sharedarray.h>
#include <boost/shared_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <string>

namespace jag
{
// fwd
namespace resources
{


class TypefaceSpecImpl
    : public ITypefaceSpec
{
public: // ITypefaceSpec
    void encoding(Char const* enc) { m_encoding = enc; }
    void file_name(Char const* file_name) { m_file_name = file_name; }
    void data(Byte const* array, UInt length);

public:
    enum TypefaceSource { IN_FILE, IN_MEMORY };

    void ensure_consistency() const;
    TypefaceSource source() const;
    SharedArray memory_data() const;
    Char const* file_name() const { return m_file_name.c_str(); }

private:
    std::string    m_encoding;
    std::string    m_file_name;

    boost::scoped_ptr<jstd::MemoryStreamOutput> m_font_data;
};

}} //namespace jag:resources


#endif//__TYPEFACESPECIMPL_H_JAG_2239__


