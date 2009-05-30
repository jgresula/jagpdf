// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __TOUNICODE_H_JAG_1304__
#define __TOUNICODE_H_JAG_1304__

// #include "resourcemanagement.h"
// #include <resources/typeman/typefacevisitor.h>
#include "indirectobjectref.h"
#include <core/jstd/unicode.h>
#include <core/generic/internal_types.h>
#include <boost/shared_ptr.hpp>

namespace jag {
namespace pdf {

// fwd
class ContentStream;
class DocWriterImpl;

class ToUnicode
    : public noncopyable
{
public:
    struct GidAndUnicode
    {
        enum { max_codepoints = 1 };
        unsigned int  gid;
        Int   codepoint[max_codepoints];
        bool operator<(GidAndUnicode const& other) const { return gid < other.gid; }
    };


    ToUnicode();
    IndirectObjectRef const& ref() const { return m_ref; }
    void output_definition(DocWriterImpl& doc, GidAndUnicode const* gids,  size_t num_gids);

private:
    void write_utf16be(std::ostringstream& ostr, Int codepoint);
    void output_char(std::ostringstream& str, GidAndUnicode const& record);
    void output_range(std::ostringstream& str, GidAndUnicode const* begin, GidAndUnicode const* end, bool is_codepoints_range);

private:
    jstd::UnicodeConverter  m_conv;
    IndirectObjectRef m_ref;
};

}} //namespace jag::pdf

#endif //__TOUNICODE_H_JAG_1304__
