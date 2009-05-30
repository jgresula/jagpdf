// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __TTFONTPARSER_H_JG_2116__
#define __TTFONTPARSER_H_JG_2116__

#include "ttstructs.h"
#include <interfaces/constants.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <boost/integer/endian.hpp>

#include <vector>
#include <bitset>
#include <string>

namespace jag
{
class IStreamInput;

namespace resources { namespace truetype
{

//////////////////////////////////////////////////////////////////////////
class TTFontParser
{
public:
    TTFontParser(IStreamInput& font_stream);
    size_t num_glyphs();
    unsigned int charcode_to_glyph_index(UInt charcode);

    /**
     * @brief loads a glyph
     * @param glyph_index glyph to load
     * @post current_glyph_size() and current_glyph_data() retrieve info
     *       associated with that glyph
     */
    void load_glyph(unsigned int glyph_index);

    /// retrieves byte size of the currently loaded glyph
    size_t current_glyph_size() const;
    /// retrieves data of the currently loaded glyph
    void const* current_glyph_data() const;

    typedef std::pair<void const*,size_t> TableData;
    /**
     * @brief loads a table
     * @param table table to load
     * @return <table data, table len>, might be <0,0> in case an optional table
     *         is not present
     */
    TableData load_table(TTTableType table);
    Char const* postscript_name();

private:
    void process_table(tt_directory_entry& entry);
    void ensure_table(TTTableType table);
    void read_table(TTTableType type, void* dest, size_t length);
    void checked_read(unsigned int offset, void* dest, size_t size, StreamOffsetOrigin offset_from=OFFSET_FROM_BEGINNING);
    void checked_read(void* dest, size_t size);
    void verify_file_checksum();
    void read_name();

private:
    IStreamInput&                m_instream;
    tt_directory_entry            m_table_dict[TT_NUM_TABLES];
    std::bitset<TT_NUM_TABLES>    m_table_present;

    std::vector<Byte>            m_loaded_glyph; // data of glyph loaded last time
    std::vector<Byte>           m_loaded_table; // data of last loaded (non-typed) table

    // tables
    boost::shared_ptr<tt_maxp>    m_maxp;
    boost::shared_ptr<tt_head>    m_head;
    boost::scoped_array<size_t>    m_loca;

    tt_cmap_fmt4*                m_cmap4;        // fixed part of format4 cmap
    tt_cmap_fmt4_arrays            m_cmap4_arr;    // array part of format4 cmap
    boost::scoped_array<Byte>    m_cmap;            // entire cmap

    std::string     m_psname;
};


}}} // namespace jag::resources::truetype

#endif //__TTFONTPARSER_H_JG_2116__
