// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __TTSTRUCT_H_JG_2032__
#define __TTSTRUCT_H_JG_2032__

#include <boost/integer/endian.hpp>
#include <interfaces/stdtypes.h>
#include <core/generic/checked_cast.h>

//#define TT_VERBOSE

#ifdef TT_VERBOSE
#   include <stdio.h>
#endif

namespace jag
{
class IStreamInput;

namespace resources { namespace truetype
{

#    pragma pack(push, normal_pack, 1)
struct tt_offset_table {
    boost::integer::big32_t        m_sfnt_version;
    boost::integer::ubig16_t    m_num_tables;
    boost::integer::ubig16_t    m_search_range;
    boost::integer::ubig16_t    m_entry_selector;
    boost::integer::ubig16_t    m_range_shift;
};

struct tt_directory_entry {
    boost::integer::ubig32_t    m_tag;
    boost::integer::ubig32_t    m_check_sum;
    boost::integer::ubig32_t    m_offset;
    boost::integer::ubig32_t    m_length;
};

struct tt_maxp {
    boost::integer::big32_t        m_version;
    boost::integer::ubig16_t    m_num_glyphs;
    boost::integer::ubig16_t    m_max_points;
    boost::integer::ubig16_t    m_max_countours;
    boost::integer::ubig16_t    m_max_composite_points;
    boost::integer::ubig16_t    m_max_composite_contours;
    boost::integer::ubig16_t    m_max_zones;
    boost::integer::ubig16_t    m_max_twilight_points;
    boost::integer::ubig16_t    m_max_storage;
    boost::integer::ubig16_t    m_max_function_defs;
    boost::integer::ubig16_t    m_max_instruction_defs;
    boost::integer::ubig16_t    m_max_stack_elements;
    boost::integer::ubig16_t    m_max_size_of_instructions;
    boost::integer::ubig16_t    m_max_component_elements;
    boost::integer::ubig16_t    m_max_component_depth;
};

struct tt_cmap_descriptor {
    boost::integer::ubig16_t    m_platform;
    boost::integer::ubig16_t    m_encoding_id;
    boost::integer::ubig32_t    m_offset;
};

struct tt_cmap_header {
    boost::integer::ubig16_t    m_version;
    boost::integer::ubig16_t    m_num_tables;
    tt_cmap_descriptor            m_tables[1];
};

struct tt_cmap_fmt4 {
    boost::integer::ubig16_t    m_format;
    boost::integer::ubig16_t    m_length;
    boost::integer::ubig16_t    m_version;
    boost::integer::ubig16_t    m_segcountx2;
    boost::integer::ubig16_t    m_search_range;
    boost::integer::ubig16_t    m_entry_selector;
    boost::integer::ubig16_t    m_range_shift;
};

struct tt_head {
    boost::integer::big32_t        m_version;
    boost::integer::big32_t        m_revision;
    boost::integer::ubig32_t    m_checksum_adjustment;
    boost::integer::ubig32_t    m_magic_number;
    boost::integer::ubig16_t    m_flags;
    boost::integer::ubig16_t    m_units_per_Em;
    Byte                        m_date_created[8];
    Byte                        m_data_modified[8];
    boost::integer::big16_t        m_x_min;    // funits
    boost::integer::big16_t        m_y_min;    // funits
    boost::integer::big16_t        m_x_max;    // funits
    boost::integer::big16_t        m_y_max;    // funits
    boost::integer::ubig16_t    m_mac_style;
    boost::integer::ubig16_t    m_lowest_rec_ppem;
    boost::integer::big16_t        m_font_direction_hint;
    boost::integer::big16_t        m_index_to_loc_fmt;
    boost::integer::big16_t        m_glyph_data_format;
};

struct tt_glyph_data {
    boost::integer::big16_t        m_number_of_contours;
    boost::integer::big16_t        m_xmin;
    boost::integer::big16_t        m_ymin;
    boost::integer::big16_t        m_xmax;
    boost::integer::big16_t        m_ymax;
};

struct tt_horizontal_header {
    boost::integer::ubig32_t    m_version;
    boost::integer::big16_t     m_ascender;
    boost::integer::big16_t     m_descender;
    boost::integer::big16_t     m_linegap;

    boost::integer::ubig16_t    m_advance_width_max;
    boost::integer::big16_t     m_min_left_side_bearing;
    boost::integer::big16_t     m_min_right_side_bearing;
    boost::integer::big16_t     m_max_extent;

    boost::integer::big16_t     m_caret_slope_rise;
    boost::integer::big16_t     m_caret_slope_run;

    boost::integer::big16_t     m_reserved1;
    boost::integer::big16_t     m_reserved2;
    boost::integer::big16_t     m_reserved3;
    boost::integer::big16_t     m_reserved4;
    boost::integer::big16_t     m_reserved5;

    boost::integer::big16_t     m_metric_data_fmt;
    boost::integer::ubig16_t    m_num_hmetrics;
};


struct tt_hor_metrics {
    boost::integer::ubig16_t    m_advance_width;
    boost::integer::big16_t     m_left_side_bearing;
};

struct tt_naming_table {
    boost::integer::ubig16_t    m_format;
    boost::integer::ubig16_t    m_count;
    boost::integer::ubig16_t    m_string_offset;
};

struct tt_name_record {
    boost::integer::ubig16_t    m_platfom_id;
    boost::integer::ubig16_t    m_encoding_id;
    boost::integer::ubig16_t    m_language_id;
    boost::integer::ubig16_t    m_name_id;
    boost::integer::ubig16_t    m_length;
    boost::integer::ubig16_t    m_offset;
};


#    pragma pack(pop, normal_pack)

struct tt_cmap_fmt4_arrays
{
    boost::integer::ubig16_t*    m_start_count;
    boost::integer::ubig16_t*    m_end_count;
    boost::integer::ubig16_t*    m_delta_count;
    boost::integer::ubig16_t*    m_range_offset;
    boost::integer::ubig16_t*    m_glyphid_array;
};


enum TTTableType {
    TT_MAXP,
    TT_CMAP,
    TT_LOCA,
    TT_HEAD,
    TT_GLYF,
    TT_NAME,
    TT_OS2,
    TT_CVT,
    TT_FPGM,
    TT_PREP,
    TT_HHEA,
    TT_HMTX,
    TT_POST,
    TT_NUM_TABLES,
};

struct TableStringToVal
{
    unsigned char*    m_str;
    TTTableType        m_id;
};

extern TableStringToVal g_table_string_to_val[TT_NUM_TABLES];
extern unsigned char g_table_str[TT_NUM_TABLES][4];

template<class T>
struct table_id
{
};

template<>
struct table_id<tt_maxp>
{
    enum { value = TT_MAXP };
};

template<>
struct table_id<tt_head>
{
    enum { value = TT_HEAD };
};



//////////////////////////////////////////////////////////////////////////
// helper functions

boost::integer::ubig32_t file_checksum(IStreamInput& instream, size_t skip_offset);
boost::integer::ubig32_t checksum(void const* data, size_t len, unsigned seed = 0);

/// converts a void pointer to a byte pointer and applies an offset optionally
inline Byte const* byte_ptr(void const* start, size_t byte_offset=0)
{
    return jag_reinterpret_cast<Byte const*>(start) + byte_offset;
}


#ifdef TT_VERBOSE
#   define TT_PRINTF printf
#else
    inline void tt_printf_noop(...) {}
#   define TT_PRINTF if (true) {} else tt_printf_noop
#endif


}}} // namespace jag::resources::truetype

#endif //__TTSTRUCT_H_JG_2032__

