// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __TTFONTMAKER_H_JG_1919__
#define __TTFONTMAKER_H_JG_1919__

#include "ttstructs.h"
#include <boost/shared_ptr.hpp>
#include <core/jstd/memory_stream.h>
#include <core/generic/noncopyable.h>

#include <boost/array.hpp>
#include <vector>
#include <map>

namespace jag
{
class ISeqStreamOutput;


namespace resources {
namespace truetype {
/**
 * @brief makes a true type font stream
 *
 * loca - 2bytes per entry, glyph data must be 2-aligned
 */
class TTFontMaker
    : public noncopyable
{
public:
    typedef std::map<Int, UInt16> CodepointToGlyph;

    TTFontMaker();
    /**
     * @brief stores a glyph data
     * @param data glyph data
     * @param data_len glyph data length
     * @param glyph_index desired index of the glyph
     */
    void add_glyph(void const* data, size_t data_len, UInt16 glyph_index);
    bool has_outlines() const;

    /**
     * @brief sets mapping from codepoint to indices
     */
    void set_codepoint_to_glyph(CodepointToGlyph const& cp2gid);

    /**
     * @brief writes the font
     * @param outstream stream the font is written to
     *
     * @pre font must contain at least one glyph
     * @post this object becomes dead, i.e. no method can be called on this
     *       object upon exiting from this method
     */
    void output(ISeqStreamOutput& outstream, bool include_cmap);

    /// adds a table to the font
    void add_table(TTTableType table, void const* table_data, size_t tabel_len);

    typedef std::pair<size_t, size_t> MemBlock;

private:
    typedef CodepointToGlyph::const_iterator MapIter;

    class RangeRec
    {
    public:
        enum Type { CONTINUOUS_CODEPOINTS, SCATTERED_CODEPOINTS };
        RangeRec(MapIter const& first, MapIter const& last, Type type);
        MapIter const& first() const { return m_first; }
        MapIter const& last() const { return m_last; }
        MapIter end() const { return ++MapIter(m_last); }
        /// num codepoints covered
        size_t len() const { return m_last->first-m_first->first+1; }
        Type type() const { return m_type; }
        size_t tt_len() const { return m_type==CONTINUOUS_CODEPOINTS ? 4 : 4 + len(); }

    private:
        MapIter    m_first;
        MapIter m_last;
        Type    m_type;

    };
    typedef std::vector<RangeRec> RangeRecords;


    void write_glyphs();
    void align_table_stream(size_t n);
    void write_cmap();
    void copy_cmap_array(std::vector<boost::integer::ubig16_t> const& data, size_t skip);
    void finish_tables_before_output();
    static void split_range(RangeRec const& rng, RangeRecords& output);
    static void write_sequence_of_ranges(
        RangeRecords& pending_ranges
        , std::vector<boost::integer::ubig16_t>& rng_arrays
        , std::vector<boost::integer::ubig16_t>& glyph_id_array
   );
    static void write_range_indices(
        RangeRec const& rng
        , std::vector<boost::integer::ubig16_t>& glyph_id_array
       );

    template<class T> T* get_table();


#   ifdef TT_VERBOSE
    static void dump_range_rec(RangeRec const& rec);
    static size_t range_rec_plus(size_t val, RangeRec const& rec);
    static void dump_range_recs(RangeRecords const& recs);
#   endif
private:
    // no alignment, performed when glyphs are copied to the table stream
    std::vector<Byte>    m_glyphs;
    std::vector<Byte>   m_hmtx;

    // table stream, tables are kept here on a 4-byte boundary
    std::vector<Byte>    m_tables;

    typedef std::map<UInt,MemBlock> GlyphMap; // glyph index -> m_glyphs[<start,end>]
    GlyphMap m_glyph_map;

    boost::array<MemBlock,TT_NUM_TABLES> m_table_records;

    CodepointToGlyph const* m_codepoint_to_glyph;
    class TableRecordWriter;
    size_t m_glyphs_byte_size;
};


}}} // namespace jag::resources::truetype

#endif //__TTFONTMAKER_H_JG_1919__
