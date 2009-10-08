// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <resources/typeman/truetype/ttfontparser.h>
#include <msg_resources.h>
#include <core/generic/assert.h>
#include <core/errlib/errlib.h>
#include <core/jstd/tracer.h>
#include <core/generic/checked_cast.h>
#include <interfaces/streams.h>

#include <cstdio>
#include <string.h>

using namespace boost::integer;

namespace jag {
namespace resources {
namespace truetype {

//////////////////////////////////////////////////////////////////////////
TTFontParser::TTFontParser(IStreamInput& font_stream)
    : m_instream(font_stream)
{
    tt_offset_table offset_table;
    m_instream.read(&offset_table, sizeof(offset_table));
    size_t num_tables = offset_table.m_num_tables;
    for (size_t i=0; i<num_tables; ++i)
    {
        tt_directory_entry entry;
        m_instream.read(&entry, sizeof(entry));
        process_table(entry);
    }

    // verify that all mandatory tables are present
    const unsigned required_tables =
          (1u << TT_CMAP)
        + (1u << TT_GLYF)
        + (1u << TT_HEAD)
        + (1u << TT_HHEA)
        + (1u << TT_HMTX)
        + (1u << TT_LOCA)
        + (1u << TT_MAXP)
        + (1u << TT_NAME)
        + (1u << TT_OS2)
        + (1u << TT_POST)
    ;
    if ((m_table_present.to_ulong()&required_tables) !=  required_tables)
        throw exception_invalid_input(msg_tt_required_table_missing()) << JAGLOC;

    verify_file_checksum();

    // verify that table sizes correspond to our knowledge
    if (
        static_cast<size_t>(m_table_dict[TT_HEAD].m_length) != sizeof(tt_head)
        || static_cast<size_t>(m_table_dict[TT_HHEA].m_length) != sizeof(tt_horizontal_header)
        || static_cast<size_t>(m_table_dict[TT_MAXP].m_length) != sizeof(tt_maxp)
        // tbd os/2
   )
    {
        throw exception_invalid_input(msg_tt_wrong_table_size()) << JAGLOC;
    }
}



//////////////////////////////////////////////////////////////////////////
void TTFontParser::verify_file_checksum()
{
    const unsigned int offset = m_table_dict[TT_HEAD].m_offset;
    const unsigned int skip_offset = offset + 8;
    Byte head_buffer[sizeof(tt_head)];
    checked_read(offset, head_buffer, sizeof(tt_head));

    tt_head const*const head = jag_reinterpret_cast<tt_head*>(head_buffer);

    if (static_cast<unsigned int>(head->m_magic_number) != 0x5f0f3cf5)
        throw exception_invalid_input(msg_tt_invalid_magic_number()) << JAGLOC;

    ubig32_t sum(file_checksum(m_instream, skip_offset));
    sum = 0xB1B0AFBA-static_cast<unsigned int>(sum);
    if (sum != head->m_checksum_adjustment)
    {
        TRACE_WRN << "TrueType checksum failed" ;
    }
}


//////////////////////////////////////////////////////////////////////////
void TTFontParser::process_table(tt_directory_entry& entry)
{
    TableStringToVal* curr = g_table_string_to_val;
    TableStringToVal const*const end = g_table_string_to_val+TT_NUM_TABLES;

    if (!entry.m_length)
        throw exception_invalid_input(msg_tt_zero_table_size()) << JAGLOC;

    for(;curr!=end; ++curr)
    {
        if (!memcmp(curr->m_str, &entry.m_tag, 4))
        {
            m_table_present.set(curr->m_id);
            m_table_dict[curr->m_id] = entry;
            break;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
void TTFontParser::read_table(TTTableType type, void* dest, size_t length)
{
    if (length != static_cast<size_t>(m_table_dict[type].m_length))
        throw exception_invalid_input(msg_tt_wrong_table_size()) << JAGLOC;

    m_instream.seek(m_table_dict[type].m_offset, OFFSET_FROM_BEGINNING);
    ULong read;
    m_instream.read(dest, length, &read);
    if (read != length)
        throw exception_invalid_input(msg_tt_cannot_read_table()) << JAGLOC;

    // verify checksum
    ubig32_t sum;
    if (type!=TT_HEAD)
    {
        sum = checksum(dest, length);
    }
    else
    {
        // head table needs a special treatment as it contains a whole file
        // checksum - for calculating the table checksum that file checksum
        // field is figured in as 0 (=ignored)
        sum = checksum(dest, 8);
        sum = checksum(byte_ptr(dest, 12), sizeof(tt_head)-12, sum);
    }

    if (m_table_dict[type].m_check_sum != sum)
    {
        // Originally, an exception was thrown here, but this was too
        // restrictive as some fonts (e.g. included in Debian distributions)
        // have wrong checksums but otherwise are ok.
        TRACE_WRN << "invalid ttf table checksum";
    }
}


//////////////////////////////////////////////////////////////////////////
void TTFontParser::ensure_table(TTTableType table)
{
    switch(table)
    {
    case TT_MAXP:
        if (!m_maxp)
        {
            JAG_PRECONDITION(m_table_present.test(TT_MAXP));
            boost::shared_ptr<tt_maxp> table(new tt_maxp);
            read_table(TT_MAXP, table.get(), sizeof(tt_maxp));
            m_maxp = table;
        }
        break;


    case TT_HEAD:
        if (!m_head)
        {
            JAG_PRECONDITION(m_table_present.test(TT_HEAD));
            boost::shared_ptr<tt_head> table(new tt_head);
            read_table(TT_HEAD, table.get(), sizeof(tt_head));
            m_head = table;
        }
        break;


    case TT_CMAP:
        if (!m_cmap)
        {
            // current strategy: entire cmap is read
            // alternative strategy: read header -> table descriptors -> format4 table
            // find out whether the second approach is worth it, namely:
            //  - amount of consumed memory
            //  - I/O access
            //
            // also it is worth considering to form own representation as the
            // lookup might suboptimal due to big endian and little endian conversions
            JAG_PRECONDITION(m_table_present.test(TT_CMAP));
            UInt len = m_table_dict[TT_CMAP].m_length;
            m_cmap.reset(new Byte[len]);
            read_table(TT_CMAP, m_cmap.get(), len);
            tt_cmap_header* cmap_head = jag_reinterpret_cast<tt_cmap_header*>(m_cmap.get());

            UInt unicode_cmap_offset = 0;
            size_t num_tables=cmap_head->m_num_tables;
            for (size_t i=0; i<num_tables; ++i)
            {
                unsigned int platform = cmap_head->m_tables[i].m_platform;
                unsigned int id = cmap_head->m_tables[i].m_encoding_id;

                // Either Microsoft, Unicode or Unicode
                if ((platform==3 && id==1) || (platform==0))
                {
                    ubig16_t* format = jag_reinterpret_cast<ubig16_t*>(
                        m_cmap.get() + cmap_head->m_tables[i].m_offset
                       );

                    if (static_cast<unsigned int>(*format) == 4u)
                    {
                        unicode_cmap_offset = cmap_head->m_tables[i].m_offset;
                        break;
                    }
                }
            }
            if (!unicode_cmap_offset)
            {
                // only cmap format 4 is supported, the client code
                // must not invoke the parser in other cases
                JAG_INTERNAL_ERROR;
            }

            m_cmap4 = jag_reinterpret_cast<tt_cmap_fmt4*>(m_cmap.get() + unicode_cmap_offset);
            unsigned int num_segments = static_cast<unsigned int>(m_cmap4->m_segcountx2) / 2u;
            m_cmap4_arr.m_end_count = jag_reinterpret_cast<ubig16_t*>(m_cmap.get() + unicode_cmap_offset + sizeof(tt_cmap_fmt4));
            m_cmap4_arr.m_start_count = m_cmap4_arr.m_end_count + num_segments + 1;
            m_cmap4_arr.m_delta_count = m_cmap4_arr.m_start_count + num_segments;
            m_cmap4_arr.m_range_offset = m_cmap4_arr.m_delta_count + num_segments;
            m_cmap4_arr.m_glyphid_array = m_cmap4_arr.m_range_offset + num_segments;
        }
        break;


    case TT_LOCA:
        if (!m_loca)
        {
            JAG_PRECONDITION(m_table_present.test(TT_LOCA));
            ensure_table(TT_MAXP);
            ensure_table(TT_HEAD);
            size_t offsets_to_read = static_cast<size_t>(m_maxp->m_num_glyphs)+1u;

            UInt loca_len = m_table_dict[TT_LOCA].m_length;
            boost::scoped_array<Byte> raw_loca(new Byte[loca_len]);
            read_table(TT_LOCA, raw_loca.get(), loca_len);

            m_loca.reset(new size_t[offsets_to_read]);
            if (!m_head->m_index_to_loc_fmt)
            {
                if (loca_len != offsets_to_read*sizeof(ubig16_t))
                    throw exception_invalid_input(msg_tt_inconsistent_loca()) << JAGLOC;

                // short offsets
                ubig16_t* offsets = jag_reinterpret_cast<ubig16_t*>(raw_loca.get());
                std::copy(offsets, offsets+offsets_to_read, m_loca.get());
                std::transform(
                    m_loca.get()
                    , m_loca.get()+offsets_to_read
                    , m_loca.get() // this is valid (see sgi doc of std::transform)
                    , std::bind1st(std::multiplies<size_t>(), 2)
                   );
            }
            else
            {
                if (loca_len != offsets_to_read*sizeof(ubig32_t))
                    throw exception_invalid_input(msg_tt_inconsistent_loca()) << JAGLOC;

                ubig32_t* offsets = jag_reinterpret_cast<ubig32_t*>(raw_loca.get());
                std::copy(offsets, offsets+offsets_to_read, m_loca.get());
            }
        }
        break;


    default:
        JAG_INTERNAL_ERROR;
    }
}



//////////////////////////////////////////////////////////////////////////
size_t TTFontParser::num_glyphs()
{
    ensure_table(TT_MAXP);
    return m_maxp->m_num_glyphs;
}

//////////////////////////////////////////////////////////////////////////
void TTFontParser::checked_read(unsigned int offset, void* dest, size_t size, StreamOffsetOrigin offset_from)
{
    if (offset)
        m_instream.seek(offset, offset_from);

    checked_read(dest, size);
}

//////////////////////////////////////////////////////////////////////////
void TTFontParser::checked_read(void* dest, size_t size)
{
    ULong to_read;
    m_instream.read(dest, size, &to_read);
    if (to_read != size)
        throw exception_io_error(msg_tt_cannot_read()) << JAGLOC;
}

//////////////////////////////////////////////////////////////////////////
void TTFontParser::load_glyph(unsigned int glyph_index)
{
    ensure_table(TT_LOCA); // implies TT_MAXP
    JAG_PRECONDITION(glyph_index < static_cast<unsigned int>(m_maxp->m_num_glyphs));

    size_t glyph_len = m_loca[glyph_index+1] - m_loca[glyph_index];
    m_loaded_glyph.resize(glyph_len);

    // zero len glyphs are allowed (e.g. space)
    if (glyph_len)
    {
        checked_read(
            static_cast<unsigned int>(m_table_dict[TT_GLYF].m_offset) + m_loca[glyph_index]
            , &m_loaded_glyph[0]
            , glyph_len
       );
    }
}


//////////////////////////////////////////////////////////////////////////
unsigned int TTFontParser::charcode_to_glyph_index(UInt charcode)
{
    JAG_PRECONDITION(charcode <= 0xffff);

    ubig16_t codepoint(static_cast<unsigned short>(charcode));
    ensure_table(TT_CMAP);

    unsigned int num_segments = static_cast<unsigned int>(m_cmap4->m_segcountx2) / 2u;

    // find the range containing charcode
    ubig16_t* found_range = std::lower_bound(
          m_cmap4_arr.m_end_count
        , m_cmap4_arr.m_end_count+num_segments
        , codepoint
   );

    ULong range_index = found_range-m_cmap4_arr.m_end_count;
    if (range_index==num_segments) {
        // reached end of range (actually should not happen,
        // as there always 'must' be a range ending with 0xffff)
        return 0;
    }

    // outside the range?
    if (m_cmap4_arr.m_start_count[range_index] > codepoint)
        return 0;


    // continuous glyph ids range?
    if (!m_cmap4_arr.m_range_offset[range_index])
        return m_cmap4_arr.m_delta_count[range_index]+codepoint;


    ubig16_t val = *(
          static_cast<unsigned int>(codepoint-m_cmap4_arr.m_start_count[range_index])
        + static_cast<unsigned int>(m_cmap4_arr.m_range_offset[range_index])/2u
        + &m_cmap4_arr.m_range_offset[range_index])
    ;

    if (val)
        return val + m_cmap4_arr.m_delta_count[range_index];

    return 0;
}

//////////////////////////////////////////////////////////////////////////
size_t TTFontParser::current_glyph_size() const
{
    return m_loaded_glyph.size();
}

//////////////////////////////////////////////////////////////////////////
void const* TTFontParser::current_glyph_data() const
{
    return m_loaded_glyph.size()
        ? &m_loaded_glyph[0]
        : static_cast<Byte*>(0)
    ;
}


//////////////////////////////////////////////////////////////////////////
TTFontParser::TableData TTFontParser::load_table(TTTableType table)
{
    switch(table)
    {
    case TT_MAXP:
        ensure_table(table);
        return TableData(m_maxp.get(), sizeof(tt_maxp));

    case TT_HEAD:
        ensure_table(table);
        return TableData(m_head.get(), sizeof(tt_head));

    // non-typed tables
    // - optional tables
    case TT_PREP:
    case TT_FPGM:
    case TT_CVT:
        if (!m_table_present[table])
            return TableData(0, 0);

    // - required tables
    case TT_NAME:
    case TT_OS2:
    case TT_HHEA:
    case TT_HMTX:
    case TT_POST:
        // length>0, ensured when reading table dictionary
        m_loaded_table.resize(m_table_dict[table].m_length);
        read_table(table, &m_loaded_table[0], m_table_dict[table].m_length);
        return TableData(&m_loaded_table[0], m_table_dict[table].m_length);

    default:
        JAG_INTERNAL_ERROR;
    };
}


//////////////////////////////////////////////////////////////////////////
void TTFontParser::read_name()
{
    //Postscript name for the font; Name ID 6 specifies a string which is used to
    //invoke a PostScript language font that corresponds to this OpenType font.
    //If no name ID 6 is present, then there is no defined method for invoking
    //this font on a PostScript interpreter.

    //OpenType fonts which include a name with name ID of 6 shall include these
    //two names with name ID 6, and characteristics as follows:

    //1. Platform: 1 [Macintosh]; Platform-specific encoding: 0 [Roman]; Language: 0 [English].
    //2. Platform: 3 [Microsoft]; Platform-specific encoding: 1 [Unicode]; Language: 0x409 [English (American)].

    //Names with name ID 6 other than the above two, if present, must be ignored.

    //When translated to ASCII, these two name strings must be identical; no longer
    //than 63 characters; and restricted to the printable ASCII subset,
    //codes 33 through 126,
    //except for the 10 characters: '[', ']', '(', ')', '{', '}', '<', '>', '/', '%'.

    tt_naming_table naming_tbl;
    checked_read(m_table_dict[TT_NAME].m_offset, &naming_tbl, sizeof(tt_naming_table));
    if (static_cast<unsigned int>(naming_tbl.m_format) != 0)
        throw exception_invalid_input(msg_tt_name_table_invalid_fmt()) << JAGLOC ;

    const size_t num_tbls = naming_tbl.m_count;
    const UInt string_storage_offset = m_table_dict[TT_NAME].m_offset + naming_tbl.m_string_offset;

    for(size_t i=0; i<num_tbls; ++i)
    {
        tt_name_record name_rec;
        checked_read(&name_rec, sizeof(tt_name_record));

        if (
               static_cast<unsigned int>(name_rec.m_platfom_id) == 3           // Microsoft
            && static_cast<unsigned int>(name_rec.m_encoding_id) == 1u         // Unicode
            && static_cast<unsigned int>(name_rec.m_language_id) == 0x409      // US English
            && static_cast<unsigned int>(name_rec.m_name_id) == 6u             // postscript name
       )
        {
            const size_t str_len2(static_cast<unsigned int>(name_rec.m_length)/2);
            std::vector<ubig16_t> str(str_len2);
            checked_read(string_storage_offset+static_cast<unsigned int>(name_rec.m_offset), &str[0], name_rec.m_length);

            m_psname.resize(str_len2);
            for (size_t i=0; i<str_len2; ++i)
                m_psname[i] = static_cast<Char>(static_cast<unsigned int>(str[i]));

            break;
        }
        else if (
               static_cast<unsigned int>(name_rec.m_platfom_id) == 1           // Macintosh
            && static_cast<unsigned int>(name_rec.m_encoding_id) == 0          // Roman
            && static_cast<unsigned int>(name_rec.m_language_id) == 0          // US English
            && static_cast<unsigned int>(name_rec.m_name_id) == 6              // postscript name
       )
        {
            m_psname.resize(name_rec.m_length);
            checked_read(string_storage_offset+static_cast<unsigned int>(name_rec.m_offset), &m_psname[0], name_rec.m_length);
            break;
        }
    }

    if (m_psname.empty())
        throw exception_invalid_input(msg_tt_no_postscript_name()) << JAGLOC;

    // check psname
    for(size_t i=0; i<m_psname.size(); i++)
    {
        if (m_psname[i]<33 || m_psname[i]>126 || strchr("[](){}<>/%", m_psname[i]))
            throw exception_invalid_input(msg_tt_psname_invalid_chars()) << JAGLOC;
    }
}

//////////////////////////////////////////////////////////////////////////
Char const* TTFontParser::postscript_name()
{
    if (m_psname.empty())
        read_name();

    return m_psname.c_str();
}



}}} // namespace jag::resources::truetype
