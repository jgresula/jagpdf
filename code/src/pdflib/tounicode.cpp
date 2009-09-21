// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "tounicode.h"
#include "docwriterimpl.h"
#include "objfmt.h"
#include <core/jstd/crt.h>
#include "contentstream.h"
#include <core/generic/assert.h>
#include <core/errlib/errlib.h>
#include <sstream>


namespace jag {
namespace pdf {




//////////////////////////////////////////////////////////////////////////
ToUnicode::ToUnicode()
    : m_conv("utf-16-be")
{}


//////////////////////////////////////////////////////////////////////////
void ToUnicode::output_definition(DocWriterImpl& doc, GidAndUnicode const* gids, size_t num_gids)
{
    JAG_PRECONDITION(gids);
    JAG_PRECONDITION(num_gids);

    std::auto_ptr<ContentStream> content_stream(doc.create_content_stream());
    // source information: range of charcode ->[unicode, ..., unicode]
    //  - maybe sorted by charcodes as it is iterator over std::map (confirm this)
    //  - now, only range charcode->unicode is provided (i.e. no support for ligatures)
    //  - utf16-be (utf-16 big endian is used)
    // mapping can be one of:
    //  - range
    //    <low> <hi> <start> ... low=start, low+1=start+1, ..., hi=start+(hi-low)
    //    <low> <hi> [ <ligature_low> <ligature_low+1> ... <ligature_hi> ]
    //  - char
    //    <charcode> <unicode>

    ObjFmtBasic& writer(content_stream->object_writer());
    writer
        .raw_text("/CIDInit /ProcSet findresource begin\n")
        .raw_text("12 dict begin\n")
        .raw_text("begincmap\n")
        .raw_text("/CIDSystemInfo\n")
        .dict_start()
        .dict_key("Registry").text_string("Adobe")
        .dict_key("Ordering").text_string("Identity")
        .dict_key("Supplement").space().output(0)
        .dict_end().raw_text(" def\n")
        .dict_key("CMapName").output("Adobe-Identity-UCS").raw_text(" def\n")
        .dict_key("CMapType").space().raw_text("2 def\n")
        .raw_text("1 begincodespacerange\n")
        .output_hex(gids[0].gid, 2) // 2 bytes
        .space()
        .output_hex(gids[num_gids-1].gid, 2)
        .raw_text("\nendcodespacerange\n")
    ;

    std::ostringstream ranges_str;
    UInt range_lines = 0;
    std::ostringstream chars;
    UInt char_lines = 0;


    // collect ranges and chars into string streams
    bool is_codepoints_range = false;
    size_t rng_start=0;
    size_t i=1;
    for(; i<num_gids; ++i)
    {
        unsigned int src_code_start = gids[rng_start].gid;
        unsigned int src_code_current = gids[i].gid;

        // check if the range of gids continues (see adobe technical note #5411, 1.4.1)
        if (src_code_current == gids[i-1].gid+1 && ((src_code_start&0xff00) == (src_code_current&0xff00)))
        {
            bool last_two_codepoints_successive = gids[i].codepoint[0] == gids[i-1].codepoint[0]+1;
            if (i == rng_start+1) // first time here within the current range?
                is_codepoints_range = last_two_codepoints_successive;

            if (is_codepoints_range != last_two_codepoints_successive)
            {
                output_range(ranges_str, gids+rng_start, gids+i, is_codepoints_range);
                rng_start = i;
                ++range_lines;
            }
        }
        else
        {
            if (i == rng_start+1)
            {
                output_char(chars, gids[rng_start]);
                ++char_lines;
            }
            else
            {
                output_range(ranges_str, gids+rng_start, gids+i, is_codepoints_range);
                ++range_lines;
            }
            rng_start = i;
        }
    }

    // process unfinished stuff
    if (rng_start+1 == i)
    {
        output_char(chars, gids[rng_start]);
        ++char_lines;
    }
    else
    {
        output_range(ranges_str, gids+rng_start, gids+i, is_codepoints_range);
        ++range_lines;
    }


    // output string streams
    if (range_lines)
    {
        writer.output(range_lines).raw_text(" beginbfrange\n");
        std::string const& ranges_string = ranges_str.str();
        writer.raw_bytes(ranges_string.c_str(), ranges_string.size());
        writer.raw_text("endbfrange\n");
    }
    if (char_lines)
    {
        writer.output(char_lines).raw_text(" beginbfchar\n");
        std::string const& char_string = chars.str();
        writer.raw_bytes(char_string.c_str(), char_string.size());
        writer.raw_text("endbfchar\n");
    }


    // finalize the unicode map
    writer
        .raw_text("endcmap\n")
        .raw_text("CMapName currentdict /CMap defineresource pop\n")
        .raw_text("end")
    ;

    //finalize
    content_stream->output_definition();
    m_ref = IndirectObjectRef(*content_stream);
}




//////////////////////////////////////////////////////////////////////////
void ToUnicode::write_utf16be(std::ostringstream& ostr, Int codepoint)
{
    char be_buffer[4];
    Int be_len = m_conv.from_codepoint(codepoint, be_buffer, 4);

    const int out_buffer_len = 11;
    char out_buffer[out_buffer_len] ; //<xxxxxxxx>
    if (be_len == 4)
    {
        jstd::snprintf_no_fail(out_buffer, out_buffer_len, "<%02x%02x%02x%02x>"
                                    , static_cast<unsigned char>(be_buffer[0])
                                    , static_cast<unsigned char>(be_buffer[1])
                                    , static_cast<unsigned char>(be_buffer[2])
                                    , static_cast<unsigned char>(be_buffer[3])
           );
    }
    else if (be_len == 2)
    {
        jstd::snprintf_no_fail(out_buffer, out_buffer_len, "<%02x%02x>"
                                    , static_cast<unsigned char>(be_buffer[0])
                                    , static_cast<unsigned char>(be_buffer[1])
           );
    }
    else
    {
        JAG_INTERNAL_ERROR;
    }

    ostr << out_buffer;
}

//////////////////////////////////////////////////////////////////////////
void ToUnicode::output_char(std::ostringstream& str, GidAndUnicode const& record)
{
    const int buffer_size = 64;
    char buffer[buffer_size]; // '<ffff> <ffff> '
    jstd::snprintf_no_fail(buffer, buffer_size, "<%04x> ", record.gid);
    str << buffer;
    write_utf16be(str, record.codepoint[0]);
    str << '\n';
}



//////////////////////////////////////////////////////////////////////////
void ToUnicode::output_range(std::ostringstream& str, GidAndUnicode const* begin, GidAndUnicode const* end, bool is_codepoints_range)
{
    JAG_PRECONDITION(end-begin > 1);
    const int buffer_size = 64;
    char buffer[buffer_size]; // '<ffff> <ffff> '
    jstd::snprintf_no_fail(buffer, buffer_size, "<%04x> <%04x> ", begin->gid, (end-1)->gid);
    str << buffer;

    if (is_codepoints_range)
    {
        write_utf16be(str, begin->codepoint[0]);
    }
    else
    {
        str << '[';
        for (GidAndUnicode const* it=begin; it!=end; ++it)
        {
            write_utf16be(str, it->codepoint[0]);
            str << ' ';
        }
        str << ']';
    }
    str << '\n';
}


}} //namespace jag::pdf
