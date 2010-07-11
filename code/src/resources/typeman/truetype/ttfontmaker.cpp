// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <resources/typeman/truetype/ttfontmaker.h>
#include <core/generic/assert.h>
#include <core/generic/math.h>
#include <core/generic/containerhelpers.h>
#include <core/generic/checked_cast.h>
#include <core/generic/internal_types.h>
#include <core/jstd/tracer.h>
#include <boost/scoped_array.hpp>
#include <math.h>
#include <numeric>
#include <string.h>

#if defined(_MSC_VER)
// disabling 'possible loss of data' as a lot of low level things is
// done here and we would end up with ugly code with many static_casts
# pragma warning(disable:4267)
# pragma warning(disable:4244)
#endif


using namespace boost::integer;

namespace jag {
namespace resources {
namespace truetype {

namespace {

// offsets to cmap format4 arrays
enum {
      START_CODE
    , END_CODE
    , DELTA
    , RANGE_OFFSET
    , NUM_ARRAYS
};

} // anonymous namespace


//////////////////////////////////////////////////////////////////////////
TTFontMaker::RangeRec::RangeRec(MapIter const& first, MapIter const& last, Type type)
    : m_first(first)
    , m_last(last)
    , m_type(type)
{
}


//////////////////////////////////////////////////////////////////////////
class TTFontMaker::TableRecordWriter
{
public:
    TableRecordWriter(TTTableType type, TTFontMaker& maker)
        : m_type(type)
        , m_maker(maker)
        , m_commited(false)
    {
        m_maker.align_table_stream(4);
        m_maker.m_table_records[m_type].first = m_maker.m_tables.size();
    }

    void commit() {
        m_maker.m_table_records[m_type].second =
            m_maker.m_tables.size()-m_maker.m_table_records[m_type].first
        ;
        m_commited = true;
    }

    ~TableRecordWriter()
    {
        JAG_ASSERT(m_commited);
    }

private:
    TTTableType     m_type;
    TTFontMaker&    m_maker;
    bool            m_commited;
};




//////////////////////////////////////////////////////////////////////////
TTFontMaker::TTFontMaker()
    : m_codepoint_to_glyph(0)
    , m_glyphs_byte_size(0)
{
    std::fill(m_table_records.begin(), m_table_records.end(), MemBlock(0,0));
}



//////////////////////////////////////////////////////////////////////////
void TTFontMaker::add_glyph(void const* data,
                            size_t data_len,
                            UInt16 glyph_index)
{
    JAG_PRECONDITION_MSG(!m_glyph_map.count(glyph_index),
                         "glyph index already in map");
    
    m_glyph_map.insert(
        std::make_pair(glyph_index,
                       MemBlock(m_glyphs.size(),
                                m_glyphs.size()+data_len)));

    Byte const* glyph_data = data_len
        ? static_cast<Byte const*>(data)
        : 0
    ;
    
    m_glyphs.insert(m_glyphs.end(), glyph_data, glyph_data+data_len);
    m_glyphs_byte_size += data_len;
}

//
//
//
bool TTFontMaker::has_outlines() const
{
    return m_glyphs_byte_size != 0;
}

 



/// aligns the table stream to a n-byte boundary
void TTFontMaker::align_table_stream(size_t n)
{
    size_t padding_bytes = 4-m_tables.size() % n;
    if (padding_bytes<4)
        m_tables.insert(m_tables.end(), padding_bytes, 0);
}



/**
 * @brief writes loca, glyf and hmtx tables, modifies also hhea
 * @post m_glyphs is released
 */
void TTFontMaker::write_glyphs()
{
    // We had to change loca format from ushort to ulong to handle
    // more glyphs. Possible optimization would be to make this
    // decision dynamically (we would need to know the total length
    // of glyphs).
    JAG_PRECONDITION_MSG(m_glyph_map.size(), "at least one glyph required");
    const UInt max_glyph_index = (--m_glyph_map.end())->first;
    const size_t loca_wsize = max_glyph_index+2;
    const size_t loca_byte_size = loca_wsize*sizeof(ubig32_t);

    boost::scoped_array<ubig32_t> loca(new ubig32_t[loca_wsize]);
    memset(loca.get(), 0, loca_byte_size);


    // write glyphs to the main stream, build loca
    TT_PRINTF("--forming loca&glyf--\n");
    TableRecordWriter glyf_record(TT_GLYF, *this);
    const size_t glyf_start = m_tables.size();
    GlyphMap::const_iterator end = m_glyph_map.end();
    for(GlyphMap::const_iterator it=m_glyph_map.begin(); it!=end; ++it)
    {
        JAG_ASSERT(!(m_tables.size()%4));
        loca[it->first] = (m_tables.size()-glyf_start);

        if (it->second.first != it->second.second) // ?not zero-size glyph
        {
            m_tables.insert(
                m_tables.end()
                , &m_glyphs[0] + it->second.first
                , &m_glyphs[0] + it->second.second
               );
            align_table_stream(4);
        }

        loca[it->first+1] = (m_tables.size()-glyf_start);
        TT_PRINTF("loca[%d]=%d\n", it->first, static_cast<unsigned int>(loca[it->first]));
        TT_PRINTF("loca[%d]=%d\n", it->first+1, static_cast<unsigned int>(loca[it->first+1]));
    }
    // for unused glyph indices, set length to zero
    for(size_t i=1; i<loca_wsize; ++i)
    {
        if (!loca[i])
            loca[i] = loca[i-1];
    }
    glyf_record.commit();
    std::vector<Byte>().swap(m_glyphs);


    // write loca
    TableRecordWriter loca_record(TT_LOCA, *this);
    Byte const* loca_bytes = jag_reinterpret_cast<Byte const*>(loca.get());
    m_tables.insert(m_tables.end(), loca_bytes, loca_bytes+loca_byte_size);
    loca_record.commit();



    // horizontal metrics
    // http://developer.apple.com/fonts/ttrefman/RM06/Chap6hmtx.html
    tt_horizontal_header* hhea = jag_reinterpret_cast<tt_horizontal_header*>(
        &m_tables[m_table_records[TT_HHEA].first]
       );
    JAG_ASSERT(m_table_records[TT_HHEA].second == sizeof(tt_horizontal_header));

    size_t hmtx_bytes = 0;
    const UInt num_glyhphs = max_glyph_index + 1;
    if (num_glyhphs <= static_cast<UInt16>(hhea->m_num_hmetrics))
    {
        hhea->m_num_hmetrics = num_glyhphs;
        hmtx_bytes = num_glyhphs * sizeof(tt_hor_metrics);
    }
    else
    {
        // leftSideBearing array length = #glyphs - hhea->m_num_hmetrics
        hmtx_bytes = static_cast<UInt16>(hhea->m_num_hmetrics) * sizeof(tt_hor_metrics) +
            2*(num_glyhphs - static_cast<UInt16>(hhea->m_num_hmetrics));
        hmtx_bytes = std::min(hmtx_bytes, m_hmtx.size());
    }

    TableRecordWriter record(TT_HMTX, *this);
    m_tables.insert(m_tables.end(), m_hmtx.begin(), m_hmtx.begin()+hmtx_bytes);
    record.commit();
}




//////////////////////////////////////////////////////////////////////////
/**
 * @brief copies one cmap parallel array to table stream
 * @param data vector containing parallel array data
 * @param offset offset of array to be copied
 */
void TTFontMaker::copy_cmap_array(std::vector<ubig16_t> const& data, size_t offset)
{
    JAG_PRECONDITION(!data.empty());
    JAG_PRECONDITION(!(data.size()%4));
    JAG_PRECONDITION(offset<4);

    Byte const*       it = byte_ptr(&data[0], offset*sizeof(ubig16_t));
    Byte const*const end = byte_ptr(&data[0], byte_size(data));

    for(; it<end; it+=4*sizeof(ubig16_t)) // data consists of 4 parallel arrays
        m_tables.insert(m_tables.end(), it, it+sizeof(ubig16_t));
}



/**
 * @brief writes a cmap
 *
 * @pre a mapping from codepoints to glyph indices
 *      must be throuhg set_codepoint_to_glyph_index()
 */
void TTFontMaker::write_cmap()
{
    if (m_codepoint_to_glyph->empty()) {
        TRACE_WRN << "no data for cmap";
        return;
    }

    // platform 3, encoding 1, format 4
    tt_cmap_header cmap_desc;
    cmap_desc.m_version = 0;
    cmap_desc.m_num_tables = 1;
    cmap_desc.m_tables[0].m_platform = 3;
    cmap_desc.m_tables[0].m_encoding_id = 1;
    cmap_desc.m_tables[0].m_offset = sizeof(tt_cmap_header);

    /*
      - phase1: find continuous ranges of codepoints
      - phase2: within found ranges find ranges of continuous glyph indices
      - phase3: try to join join several ranges with scattered codepoints to optimize the final size
    */


    // phase1
    RangeRecords ranges1;
    MapIter it=m_codepoint_to_glyph->begin();
    MapIter map_end = m_codepoint_to_glyph->end();
    MapIter first = it;
    TT_PRINTF("cmap - phase1 - detecting code ranges\n");
    TT_PRINTF("%04x -> %d\n", it->first, it->second);
    MapIter last = it++;
    for(; it!=map_end; ++it)
    {
        if (it->first==last->first+1)
        {
            last = it;
        }
        else
        {
            ranges1.push_back(RangeRec(first, last, RangeRec::SCATTERED_CODEPOINTS));
            TT_PRINTF(">len: %d\n", RangeRec(first, last, RangeRec::SCATTERED_CODEPOINTS).len());
            first = it;
            last = it;
        }
        TT_PRINTF("%04x -> %d\n", it->first, it->second);
    }
    ranges1.push_back(RangeRec(first, last, RangeRec::SCATTERED_CODEPOINTS));
    TT_PRINTF(">len: %d\n", RangeRec(first, last, RangeRec::SCATTERED_CODEPOINTS).len());
#ifdef TT_VERBOSE
    TTFontMaker::dump_range_recs(ranges1);
#endif


    // phase2
    TT_PRINTF("\ncmap - phase2 - detecting ranges of glyph indices\n");
    RangeRecords ranges2;
    ranges2.reserve(ranges1.size());
    for(RangeRecords::const_iterator it = ranges1.begin(); it!=ranges1.end(); ++it)
    {
        // split only ranges with length > 1
        if (it->len() > 1)
        {
            TTFontMaker::split_range(*it, ranges2);
        }
        else
        {
            ranges2.push_back(*it);
        }
    }

#ifdef TT_VERBOSE
    TTFontMaker::dump_range_recs(ranges2);
#endif


    // phase3
    TT_PRINTF("\ncmap - phase3 - joining ranges\n");
    std::vector<ubig16_t> rng_arrays;
    // size is 1 in order to allow zero range_offset
    // which indicates that no glyph_id_array is not used
    // for given range
    std::vector<ubig16_t> glyph_id_array(1);
    glyph_id_array[0] = 0;


    RangeRecords::const_iterator end = ranges2.end();
    size_t pending_range_size = 0;
    RangeRecords& pending_ranges(ranges1); // reuse
    pending_ranges.clear();
    for(RangeRecords::const_iterator it = ranges2.begin(); it!=end; ++it)
    {
        if (RangeRec::CONTINUOUS_CODEPOINTS == it->type() || it->len() == 1)
        {
            if (pending_range_size)
            {
                TTFontMaker::write_sequence_of_ranges(pending_ranges, rng_arrays, glyph_id_array);
                pending_range_size = 0;
            }

#           ifdef TT_VERBOSE
            TT_PRINTF("writing continuous: ");
            dump_range_rec(*it);
#           endif

            const size_t offset = rng_arrays.size();
            rng_arrays.insert(rng_arrays.end(), NUM_ARRAYS, 0);
            rng_arrays[offset+START_CODE] = it->first()->first;
            rng_arrays[offset+END_CODE] = it->last()->first;
            rng_arrays[offset+DELTA] = it->first()->second - it->first()->first;
        }
        else
        {
            if (!pending_range_size)
            {
                pending_ranges.push_back(*it);
                pending_range_size = 4 + it->len();
            }
            else
            {
                const size_t this_rng_size = 4 + it->len();
                JAG_ASSERT(it->last()->first > pending_ranges.back().first()->first);
                const size_t joined_rng_size = it->last()->first - pending_ranges.back().last()->first+ pending_range_size;
                if (pending_range_size+this_rng_size > joined_rng_size)
                {
                    pending_ranges.push_back(*it);
                    pending_range_size = joined_rng_size;
                }
                else
                {
                    TTFontMaker::write_sequence_of_ranges(pending_ranges, rng_arrays, glyph_id_array);
                    pending_ranges.push_back(*it);
                    pending_range_size = 4 + it->len();
                }
            }
        }
    }
    if (pending_range_size)
        TTFontMaker::write_sequence_of_ranges(pending_ranges, rng_arrays, glyph_id_array);


    // add the last 0xffff segment and adjust range offsets
    size_t offset = rng_arrays.size();
    rng_arrays.insert(rng_arrays.end(), NUM_ARRAYS, 0);
    rng_arrays[offset+START_CODE] = 0xffff;
    rng_arrays[offset+END_CODE] = 0xffff;


    // postprocess RANGE_OFFSET array (offsets starts from the byte that stores the actual offset)
    const unsigned rng_arrays_size = static_cast<unsigned>(rng_arrays.size());
    JAG_ASSERT(!(rng_arrays_size%NUM_ARRAYS));
    const unsigned num_ranges = rng_arrays_size / NUM_ARRAYS;
    unsigned curr_range_offset = num_ranges*sizeof(ubig16_t);
    for (size_t i=RANGE_OFFSET; i<rng_arrays_size; i+=NUM_ARRAYS, curr_range_offset-=sizeof(ubig16_t))
    {
        if (rng_arrays[i])
            rng_arrays[i]+=curr_range_offset;
    }

    // write ranges
    tt_cmap_fmt4 fmt4;
    fmt4.m_format = 4;
    fmt4.m_length =
        sizeof(tt_cmap_fmt4)
        + sizeof(ubig16_t)                // reserved field
        + byte_size(rng_arrays)         // 4 parallel arrays
        + byte_size(glyph_id_array);    // glyph ids
    fmt4.m_version = 0;
    fmt4.m_segcountx2 = num_ranges*2;
    unsigned tmp = slow_log2(num_ranges);
    fmt4.m_search_range = 2*(tmp*tmp);
    fmt4.m_entry_selector = slow_log2(static_cast<UInt16>(fmt4.m_search_range)/2);
    fmt4.m_range_shift = fmt4.m_segcountx2    - fmt4.m_search_range;


    // write cmap to tables stream
    align_table_stream(4);
    m_tables.reserve(m_tables.size()+static_cast<UInt16>(fmt4.m_length)+sizeof(tt_cmap_header));

    // write cmap header
    m_table_records[TT_CMAP].first = m_tables.size();
    m_tables.insert(m_tables.end(), byte_ptr(&cmap_desc), byte_ptr(&cmap_desc, sizeof(tt_cmap_header)));

    // write static part of cmap fmt4
    m_tables.insert(m_tables.end(), byte_ptr(&fmt4), byte_ptr(&fmt4, sizeof(tt_cmap_fmt4)));

    // write 4 parallel arrays of cmap fmt4
    copy_cmap_array(rng_arrays, END_CODE);
    m_tables.insert(m_tables.end(), sizeof(ubig16_t), 0);  //reserved field
    copy_cmap_array(rng_arrays, START_CODE);
    copy_cmap_array(rng_arrays, DELTA);
    copy_cmap_array(rng_arrays, RANGE_OFFSET);

    // write glyph id array
    const size_t glyph_id_byte_len = sizeof(ubig16_t)*glyph_id_array.size();
    if (glyph_id_byte_len)
    {
        m_tables.insert(
            m_tables.end()
            , byte_ptr(&glyph_id_array[0])
            , byte_ptr(&glyph_id_array[0], glyph_id_byte_len)
           );
    }

    m_table_records[TT_CMAP].second = m_tables.size()-m_table_records[TT_CMAP].first;
    JAG_ASSERT(static_cast<UInt16>(fmt4.m_length)+sizeof(tt_cmap_header) == m_table_records[TT_CMAP].second);
}


/**
 * @brief flatten a sequence of ranges into single one and write it tt structs
 * @param pending_ranges sequence of ranges to write
 * @param range_nr number of range being processed
 * @param rng_arrays 4 parallel arrays the range is written into
 * @param glyph_id_array glyph indices corresponding to the range being written are stored here
 * @post pending_ranges.emtpy(), rng_arrays extended by 4 items,
 *       glyph_id_array extended by size of the flattened range
 */
void TTFontMaker::write_sequence_of_ranges(
      RangeRecords& pending_ranges
    , std::vector<ubig16_t>& rng_arrays
    , std::vector<ubig16_t>& glyph_id_array
)
{
    JAG_PRECONDITION(!pending_ranges.empty());
#   ifdef TT_VERBOSE
    TT_PRINTF("writing pending: ");
    dump_range_recs(pending_ranges);
#   endif

    const size_t offset = rng_arrays.size();
    rng_arrays.insert(rng_arrays.end(), NUM_ARRAYS, 0);
    rng_arrays[offset+START_CODE] = pending_ranges.front().first()->first;
    rng_arrays[offset+END_CODE] = pending_ranges.back().last()->first;
    rng_arrays[offset+RANGE_OFFSET] = sizeof(ubig16_t)*static_cast<unsigned short>(glyph_id_array.size());

    const size_t num_indices =
          static_cast<size_t>(rng_arrays[offset+END_CODE])
        - static_cast<size_t>(rng_arrays[offset+START_CODE])
        + 1
    ;

    glyph_id_array.reserve(glyph_id_array.size() + num_indices);

    const size_t num_ranges1 = pending_ranges.size()-1;
    for (size_t i=0; i<num_ranges1; ++i)
    {
        TTFontMaker::write_range_indices(pending_ranges[i], glyph_id_array);
        JAG_ASSERT(pending_ranges[i+1].first()->first > pending_ranges[i].last()->first);
        size_t gap_size = pending_ranges[i+1].first()->first-pending_ranges[i].last()->first-1;
        std::fill_n(std::back_inserter(glyph_id_array), gap_size, 0);
    }
    // the last range
    TTFontMaker::write_range_indices(pending_ranges.back(), glyph_id_array);
    JAG_ASSERT(num_indices == glyph_id_array.size()-static_cast<size_t>(rng_arrays[offset+RANGE_OFFSET])/sizeof(ubig16_t));
    pending_ranges.clear();
}


/// appends indices contained in the given range to an array
void TTFontMaker::write_range_indices(
      RangeRec const& rng
    , std::vector<boost::integer::ubig16_t>& glyph_id_array)
{
    JAG_ASSERT(rng.type() == RangeRec::SCATTERED_CODEPOINTS);
    MapIter last = rng.end();
    MapIter it = rng.first();
    for (; it!=last; ++it)
    {
        glyph_id_array.push_back(it->second);
    }
}



/**
 * @brief attempts to splits a range of codepoints
 * @param rng_rec range to split
 * @param output where to store the resulting ranges
 */
void TTFontMaker::split_range(RangeRec const& rng_rec, RangeRecords& output)
{
    //  - word length of a range of size N>0 with
    //    - continuous codepoints: 4
    //    - scattered glyph indices: 4+N

    size_t cont_range_len=1;
    MapIter it = rng_rec.first();
    MapIter end = rng_rec.end();
    MapIter prev_it = it++;
    MapIter cont_rng_start;
    for(size_t rng_i=1; it!=end; ++it, ++rng_i)
    {
        if (it->second == prev_it->second+1)
        {
            if (2== ++cont_range_len)
                cont_rng_start = prev_it;
        }
        else
        {   // glyph index range is not continuous, check if we rounded up
            // some continuous sequence so far
            if (cont_range_len>1)
            {
                // find out whether it is good to split the range, if so
                // then solve the following cases:
                // i)   - cut range from left - 2 ranges
                // ii)  - split range in the middle - 3 ranges
                const size_t rng_left = rng_i-cont_range_len;
                const size_t rng_right = rng_rec.len() -rng_i;
                const size_t old_range_ttlen = 4 + rng_rec.len();
                const size_t new_range_ttlen = (rng_left+(rng_left?4:0)) + 4 + (rng_right+(rng_right?4:0));
                if (old_range_ttlen > new_range_ttlen)
                {
                    JAG_ASSERT(rng_left+cont_range_len+rng_right == rng_rec.len());
                    if (rng_left>1)
                        output.push_back(RangeRec(rng_rec.first(), --MapIter(cont_rng_start), RangeRec::SCATTERED_CODEPOINTS));

                    output.push_back(RangeRec(cont_rng_start,  --MapIter(it), RangeRec::CONTINUOUS_CODEPOINTS));

                    if (rng_right>1)
                        TTFontMaker::split_range(RangeRec(it, --MapIter(end), RangeRec::SCATTERED_CODEPOINTS), output);

                    return;
                }
                else
                {
                    // do not split this time
                    cont_range_len=1;
                    prev_it = it;
                    --prev_it;
                }
            }
        }
        ++prev_it;
    }

    if (cont_range_len > 1)
    {
        const size_t rng_left = rng_rec.len()-cont_range_len;
        const size_t old_range_ttlen = 4 + rng_rec.len();
        const size_t new_range_ttlen = (rng_left+(rng_left?4:0)) + 4;
        if (old_range_ttlen > new_range_ttlen)
        {
            JAG_ASSERT(rng_left+cont_range_len == rng_rec.len());
            if (rng_left>1)
                output.push_back(RangeRec(rng_rec.first(), --MapIter(cont_rng_start), RangeRec::SCATTERED_CODEPOINTS));

            output.push_back(RangeRec(cont_rng_start,  rng_rec.last(), RangeRec::CONTINUOUS_CODEPOINTS));
        }
    }
    else
    {
        output.push_back(rng_rec);
    }
}



//////////////////////////////////////////////////////////////////////////
void TTFontMaker::add_table(TTTableType table, void const* table_data, size_t table_len)
{
    JAG_PRECONDITION_MSG(
           table!=TT_GLYF
        && table!=TT_LOCA
        && table!=TT_CMAP
        , "these tables are built by this class"
   );

    bool copy_to_table_stream = true;

    // pre action
    switch(table)
    {
    case TT_HMTX:
        m_hmtx.resize(table_len);
        std::copy(byte_ptr(table_data), byte_ptr(table_data,table_len), &m_hmtx[0]);
        copy_to_table_stream = false;
        break;

    case TT_POST:
        table_len = 32;
        break;

    default:
        ;
    }

    if (copy_to_table_stream)
    {
        align_table_stream(4);
        m_table_records[table] = std::make_pair(m_tables.size(), table_len);
        Byte const* byte_data = static_cast<Byte const*>(table_data);
        m_tables.insert(m_tables.end(), byte_data, byte_data+table_len);
    }

    // post action
    switch(table)
    {
    case TT_POST: {
            JAG_ASSERT(table_len==32);
            Byte *const table = &m_tables[m_table_records[TT_POST].first];
            static const Byte fmt[4] = { 0, 3, 0, 0 };
            memcpy(table, fmt, 4); // change version to 0x00030000
            memset(table+16, 0, 16); // no information provided to ps driver regarding mem usage
        }
        break;

    case TT_HEAD: {
            JAG_ASSERT(table_len == sizeof(tt_head));
            tt_head* head = jag_reinterpret_cast<tt_head*>(&m_tables[m_table_records[TT_HEAD].first]);
            head->m_index_to_loc_fmt = 1;
//            head->m_index_to_loc_fmt = 0; short
        }
        break;

    default:
        ;
    }

}

namespace
{


//////////////////////////////////////////////////////////////////////////
bool is_table_present(TTFontMaker::MemBlock const& record)
{
    return record.second ? true : false;
}

}

//////////////////////////////////////////////////////////////////////////
template<class T>
T* TTFontMaker::get_table()
{
    JAG_ASSERT(m_table_records[table_id<T>::value].second == sizeof(T));
    return jag_reinterpret_cast<T*>(&m_tables[m_table_records[table_id<T>::value].first]);
}

//////////////////////////////////////////////////////////////////////////
void TTFontMaker::finish_tables_before_output()
{
    // MAXP - #glyphs
    tt_maxp* maxp(get_table<tt_maxp>());
//    const unsigned int maxp_num_glyphs = m_table_records[TT_LOCA].second/sizeof(ubig16_t) - 1;
    const unsigned int maxp_num_glyphs = m_table_records[TT_LOCA].second/sizeof(ubig32_t) - 1;
    maxp->m_num_glyphs = maxp_num_glyphs;

    // HEAD - file checksum
    tt_head* head(get_table<tt_head>());
    head->m_checksum_adjustment = 0;
}


//////////////////////////////////////////////////////////////////////////
void TTFontMaker::output(ISeqStreamOutput& outstream, bool include_cmap)
{
    // ... that the length of a table must be a multiple of four bytes. While this
    //is not a requirement for the TrueType scaler itself, it is suggested that all
    //tables begin on four byte boundaries, and pad any remaining space between
    //tables with zeros. The length of all tables should be recorded in the table
    //directory with their actual length.

    TT_PRINTF("outputting font\n");

    write_glyphs();
    if (include_cmap)
        write_cmap();

    align_table_stream(4);
    finish_tables_before_output();

    ubig32_t sum(0);

    // offset table
    tt_offset_table offset_table;
    offset_table.m_sfnt_version = 0x00010000;
    offset_table.m_num_tables = std::count_if(m_table_records.begin(), m_table_records.end(), &is_table_present);
    offset_table.m_search_range = 16*(1<<slow_log2(offset_table.m_num_tables));
    offset_table.m_entry_selector = slow_log2(static_cast<UInt16>(offset_table.m_search_range)/16u);
    offset_table.m_range_shift = static_cast<UInt16>(offset_table.m_num_tables)*16u - static_cast<UInt16>(offset_table.m_search_range);

    sum = checksum(&offset_table, sizeof(tt_offset_table), 0);
    outstream.write(&offset_table, sizeof(tt_offset_table));


    // table directory entries
    const size_t start_offset =
        sizeof(tt_offset_table)
        + static_cast<unsigned short>(offset_table.m_num_tables) * sizeof(tt_directory_entry);
    JAG_ASSERT(!(start_offset%4));
    for(unsigned int table_i=0; table_i<TT_NUM_TABLES; ++table_i)
    {
        const size_t table_len = m_table_records[table_i].second;
        if (table_len)
        {
            // form a table directory entry
            tt_directory_entry dir_entry;
            memcpy(&dir_entry.m_tag, g_table_str[table_i], 4);
            const size_t table_offset = m_table_records[table_i].first;
            dir_entry.m_check_sum = checksum(&m_tables[table_offset], table_len);
            dir_entry.m_offset = start_offset + table_offset;
            dir_entry.m_length = table_len;
            sum = checksum(&dir_entry, sizeof(tt_directory_entry), sum);
            outstream.write(&dir_entry, sizeof(tt_directory_entry));
            TT_PRINTF("table %.4s, offset=%d, len=%d\n"
                       , g_table_str[table_i]
                       , static_cast<unsigned int>(dir_entry.m_offset)
                       , table_len
               );
        }
    }


    // finalize the file checksum
    JAG_ASSERT(!(m_tables.size()%4));
    sum = checksum(&m_tables[0], m_tables.size(), sum);
    tt_head* head(get_table<tt_head>());
    head->m_checksum_adjustment = 0xB1B0AFBA - static_cast<unsigned int>(sum);


    // copy tables to the output stream
    outstream.write(&m_tables[0], m_tables.size());
    std::vector<Byte>().swap(m_tables);
}


//////////////////////////////////////////////////////////////////////////
void TTFontMaker::set_codepoint_to_glyph(CodepointToGlyph const& cp2gid)
{
    m_codepoint_to_glyph = &cp2gid;
}




//////////////////////////////////////////////////////////////////////////
// diagnostics
//////////////////////////////////////////////////////////////////////////


#ifdef TT_VERBOSE
void TTFontMaker::dump_range_rec(RangeRec const& rec)
{
    TT_PRINTF("len: %d, type: %d, first: %04x->%d, last: %04x->%d\n"
        , rec.len(), rec.type()
        , rec.first()->first, rec.first()->second
        , rec.last()->first, rec.last()->second);
}

size_t TTFontMaker::range_rec_plus(size_t val, RangeRec const& rec)
{
    return val + rec.tt_len();
}

void TTFontMaker::dump_range_recs(RangeRecords const& recs)
{
    TT_PRINTF("#ranges: %d\n", recs.size());
    std::for_each(recs.begin(), recs.end(), &TTFontMaker::dump_range_rec);
    TT_PRINTF("#tt_len: %d\n", std::accumulate(recs.begin(), recs.end(), 0, &TTFontMaker::range_rec_plus));
}

#endif



}}} // namespace jag::resources::truetype
