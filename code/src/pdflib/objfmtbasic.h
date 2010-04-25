// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __OBJFFMTBASIC_H__
#define __OBJFFMTBASIC_H__

#include "resourcenames.h"
#include "defines.h"
#include <interfaces/stdtypes.h>
#include <core/generic/noncopyable.h>
#include <core/jstd/unicode.h>
#include <string>

#ifdef JAG_DEBUG
# include <stack>
#endif

namespace jag {
// fwd
class ISeqStreamOutput;
namespace jstd { class trans_affine_t; }

namespace pdf {

// fwd
class EncryptionStream;

// graphics operators
//
// convention for non-letters:
//  * -> star
//  ' -> quote
//  " -> double quote
//
enum GraphicsOperator {
    // OPCAT_GENERAL_GRAPHICS_STATE
    OP_w = 0,
    OP_J,
    OP_j,
    OP_M,
    OP_d,
    OP_ri,
    OP_i,
    OP_gs,
    // OPCAT_SPECIAL_GRAPHICS_STATE
    OP_q,
    OP_Q,
    OP_cm,
    // OPCAT_PATH_CONSTRUCTION
    OP_m,
    OP_l,
    OP_c,
    OP_v,
    OP_y,
    OP_h,
    OP_re,
    // OPCAT_PATH_PAINTING
    OP_S,
    OP_s,
    OP_f,
    OP_F,
    OP_f_star,
    OP_B,
    OP_B_star,
    OP_b,
    OP_b_star,
    OP_n,
    // OPCAT_CLIPPING_PATHS
    OP_W,
    OP_W_star,
    // OPCAT_TEXT_OBJECTS
    OP_BT,
    OP_ET,
    // OPCAT_TEXT_STATE
    OP_Tc,
    OP_Tw,
    OP_Tz,
    OP_TL,
    OP_Tf,
    OP_Tr,
    OP_Ts,
    // OPCAT_TEXT_POSITIONING
    OP_Td,
    OP_TD,
    OP_Tm,
    OP_T_star,
    // OPCAT_TEXT_SHOWING
    OP_Tj,
    OP_TJ,
    OP_quote,
    OP_double_quote,
    // OPCAT_TYPE_3_FONTS
    OP_d0,
    OP_d1,
    // OPCAT_COLOR
    OP_CS,
    OP_cs,
    OP_SC,
    OP_SCN,
    OP_sc,
    OP_scn,
    OP_G,
    OP_g,
    OP_RG,
    OP_rg,
    OP_K,
    OP_k,
    // OPCAT_SHADING_PATTERNS
    OP_sh,
    // OPCAT_INLINE_IMAGES
    OP_BI,
    OP_ID,
    OP_EI,
    // OPCAT_MARKED_CONTENT
    OP_MP ,
    OP_DP ,
    OP_BMC,
    OP_BDC,
    OP_EMC,
    // OPCAT_COMPATIBILITY
    OP_BX,
    OP_EX,
    // OPCAT_XOBJECTS
    OP_Do_image,
    OP_Do_mask
};


/**
 * @brief basic formatting functionality
 *
 * Encryption is supported through encryption_stream(). When a method performs
 * encryption then it is explicitly stated in its documentation.
 *
 * @todo check constraints Table c.1 (pg 920)
 */
class ObjFmtBasic
    : public noncopyable
{
public:
    ObjFmtBasic(ISeqStreamOutput&, jstd::UnicodeConverterStream&  utf8_to_16be);
    ~ObjFmtBasic();
    void flush() {};
    void encryption_stream(EncryptionStream* enc_stream);

public:
    ObjFmtBasic& array_start();
    ObjFmtBasic& array_end();
    ObjFmtBasic& dict_start();
    ObjFmtBasic& dict_end();
    ObjFmtBasic& dict_key(Char const* key);
    ObjFmtBasic& space();

    ObjFmtBasic& output(Int value);
    ObjFmtBasic& output(UInt value);
    ObjFmtBasic& output(Double value);
    ObjFmtBasic& output(Char const* value);
    ObjFmtBasic& output_bool(bool value);
    ObjFmtBasic& output_hex(UInt value, size_t bytes);
    ObjFmtBasic& output(jstd::trans_affine_t const& value);
    ObjFmtBasic& name(Char const* value);
    ObjFmtBasic& rectangle(Double llx, Double lly, Double urx, Double ury);
    ObjFmtBasic& null();

    ObjFmtBasic& graphics_op(GraphicsOperator op);
    ObjFmtBasic& raw_text(Char const* value);
    ObjFmtBasic& raw_bytes(void const* data, size_t length);
    ObjFmtBasic& stream_data(void const* data, size_t length);

    ObjFmtBasic& text_string(Char const* txt, size_t length, bool is_utf8=true);
    ObjFmtBasic& text_string(std::string const& txt, bool is_utf8=true);
    ObjFmtBasic& text_string(Char const* txt);
    ObjFmtBasic& text_string_hex(Char const* txt, size_t length);

    ObjFmtBasic& unenc_text_string(Char const* txt, size_t length);
    ObjFmtBasic& unenc_text_string(Char const* txt);
    ObjFmtBasic& unenc_text_string_hex(Char const* txt, size_t length);

    ObjFmtBasic& string_object_2b(UInt16 const* txt, size_t length);
    ObjFmtBasic& string_object_2b(Char const* start, Char const* end);

    template<class HANDLE>
    ObjFmtBasic& output_resource(HANDLE const& handle);


    // !! this is actually very dangerous due to type conversions
    // if really need, provide home-grown bool type
    //    ObjectWriter& output(bool value);

public:
    unsigned operator_categories() const { return m_operator_categories; }
    void copy_to(ObjFmtBasic& other) const;

public:
    //
    // states of the graphics state machine
    //
    enum ContentStateId
    {
        STATE_PAGE_DESCRIPTION,
        STATE_TEXT_OBJECT,
        STATE_CLIPPING_PATH,
        STATE_PATH_OBJECT,
        NUM_STATES
    };


private:
    void text_string_internal(ISeqStreamOutput& out_stream, Char const* txt, size_t length);
    void text_string_hex_internal(ISeqStreamOutput& out_stream, Char const* txt, size_t length);

    ISeqStreamOutput* m_stream;
    ISeqStreamOutput* m_enc_stream;            //never gets NULL
    EncryptionStream* m_enc_stream_ctrl;

    jstd::UnicodeConverterStream& m_utf8_to_16be;
    ContentStateId m_state;
    unsigned m_operator_categories;

    // checks output consistency as far as dictionaries and arrays are concerned
#ifdef JAG_DEBUG
    enum OUTPUT_TYPE { ARRAY, DICT };
    std::stack<OUTPUT_TYPE> m_consistency_stack;
#endif
};


/**
* @brief writes current date (according to pdf spec 3.8.3)
*
* Universal time (UT) is not written.
*
* usage:
* @code
* const char* data = DateWriter().date();
* @endcode
*/

class DateWriter
{
public:
    DateWriter();
    char const* date() const { return m_date; }
private:
    enum { BUFFER_SIZE = 22 };
    char m_date[BUFFER_SIZE];
};


/**
 * @brief Outputs an array.
 *
 * @param arr pointer to the first element
 * @param size number of elements
 * @param fmt formatter to use
 */
template<class T>
void output_array(T const* arr, size_t size, ObjFmtBasic& fmt)
{
    fmt.array_start();
    if (size-- > 0)
    {
        size_t i=0;
        while (i<size)
            fmt.output(arr[i++]).space();
        fmt.output(arr[i]);
    }
    fmt.array_end();

}

//
//
//
template<class HANDLE>
ObjFmtBasic& ObjFmtBasic::output_resource(HANDLE const& handle)
{
    ResNameInfo res_rec = get_resource_name_record<HANDLE>();
    m_stream->write(res_rec.first, res_rec.second);
    output(handle.id()); //resources numbering in the output is one-based
    return *this;
}


}} //namespace jag::pdf


#endif //__OBJFFMTBASIC_H__
