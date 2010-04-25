// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include "objfmtbasic.h"
#include "encryption_stream.h"

#include <interfaces/stdtypes.h>
#include <core/generic/scopeguard.h>
#include <core/generic/macros.h>
#include <core/generic/assert.h>
#include <core/jstd/crt.h>
#include <core/jstd/streamhelpers.h>
#include <core/jstd/tracer.h>
#include <core/errlib/msg_writer.h>
#include <core/errlib/errlib.h>
#include <core/generic/assert.h>
#include <core/jstd/tracer.h>
#include <core/jstd/transaffine.h>
#include <core/generic/floatpointtools.h>
#include <core/jstd/memory_stream.h>
#include <interfaces/streams.h>
#include <core/generic/minmax.h>
#include <core/generic/checked_cast.h>
#include <boost/scoped_array.hpp>

#include <ctime>
#include <cstdio>
#include <limits>
#include <string>
#include <exception>


using namespace jag::jstd;



namespace jag {
namespace pdf {


/// anonymous
namespace
{

  // ---------------------------------------------------------------------------
  //                Operators and their categories
  //

  //
  //
  //
  struct OperatorRecord
  {
      char const*  name;
      int          length;
      unsigned     category;
  };


  //
  //
  //
  const OperatorRecord s_operators[] = {
      // OPCAT_GENERAL_GRAPHICS_STATE
      {" w ",  3, OPCAT_GENERAL_GRAPHICS_STATE}, // OP_w = 0
      {" J ",  3, OPCAT_GENERAL_GRAPHICS_STATE}, // OP_J
      {" j ",  3, OPCAT_GENERAL_GRAPHICS_STATE}, // OP_j
      {" M ",  3, OPCAT_GENERAL_GRAPHICS_STATE}, // OP_M
      {" d ",  3, OPCAT_GENERAL_GRAPHICS_STATE}, // OP_d
      {" ri ", 4, OPCAT_GENERAL_GRAPHICS_STATE}, // OP_ri
      {" i ",  3, OPCAT_GENERAL_GRAPHICS_STATE}, // OP_i
      {" gs ", 4, OPCAT_GENERAL_GRAPHICS_STATE}, // OP_gs
      // OPCAT_SPECIAL_GRAPHICS_STATE
      {"q ",   2, OPCAT_SPECIAL_GRAPHICS_STATE}, // OP_q,
      {"Q ",   2, OPCAT_SPECIAL_GRAPHICS_STATE}, // OP_Q,
      {" cm ", 4, OPCAT_SPECIAL_GRAPHICS_STATE}, // OP_cm,
      // OPCAT_PATH_CONSTRUCTION
      {" m ",  3, OPCAT_PATH_CONSTRUCTION}, // OP_m,
      {" l ",  3, OPCAT_PATH_CONSTRUCTION}, // OP_l,
      {" c ",  3, OPCAT_PATH_CONSTRUCTION}, // OP_c,
      {" v ",  3, OPCAT_PATH_CONSTRUCTION}, // OP_v,
      {" y ",  3, OPCAT_PATH_CONSTRUCTION}, // OP_y,
      {"h ",   2, OPCAT_PATH_CONSTRUCTION}, // OP_h,
      {" re ", 4, OPCAT_PATH_CONSTRUCTION}, // OP_re,
      // OPCAT_PATH_PAINTING
      {"S ",   2, OPCAT_PATH_PAINTING}, // OP_S,
      {"s ",   2, OPCAT_PATH_PAINTING}, // OP_s,
      {"f ",   2, OPCAT_PATH_PAINTING}, // OP_f,
      {"F ",   2, OPCAT_PATH_PAINTING}, // OP_F,
      {"f* ",  3, OPCAT_PATH_PAINTING}, // OP_f_star,
      {"B ",   2, OPCAT_PATH_PAINTING}, // OP_B,
      {"B* ",  3, OPCAT_PATH_PAINTING}, // OP_B_star,
      {"b ",   2, OPCAT_PATH_PAINTING}, // OP_b,
      {"b* ",  3, OPCAT_PATH_PAINTING}, // OP_b_star,
      {"n ",   2, OPCAT_PATH_PAINTING}, // OP_n
      // OPCAT_CLIPPING_PATHS
      {"W ",   2, OPCAT_CLIPPING_PATHS}, // OP_W,
      {"W* ",  3, OPCAT_CLIPPING_PATHS}, // OP_W_star,
      // OPCAT_TEXT_OBJECTS
      {"BT ",  3, OPCAT_TEXT_OBJECTS}, // OP_BT,
      {"ET ",  3, OPCAT_TEXT_OBJECTS}, // OP_ET,
      // OPCAT_TEXT_STATE
      {" Tc ", 4, OPCAT_TEXT_STATE}, // OP_Tc,
      {" Tw ", 4, OPCAT_TEXT_STATE}, // OP_Tw,
      {" Tz ", 4, OPCAT_TEXT_STATE}, // OP_Tz,
      {" TL ", 4, OPCAT_TEXT_STATE}, // OP_TL,
      {" Tf ", 4, OPCAT_TEXT_STATE}, // OP_Tf,
      {" Tr ", 4, OPCAT_TEXT_STATE}, // OP_Tr,
      {" Ts ", 4, OPCAT_TEXT_STATE}, // OP_Ts
      // OPCAT_TEXT_POSITIONING
      {" Td ", 4, OPCAT_TEXT_POSITIONING}, // OP_Td,
      {" TD ", 4, OPCAT_TEXT_POSITIONING}, // OP_TD,
      {" Tm ", 4, OPCAT_TEXT_POSITIONING}, // OP_Tm,
      {"T* ",  3, OPCAT_TEXT_POSITIONING}, // OP_T_star, -
      // OPCAT_TEXT_SHOWING
      {" Tj ", 4, OPCAT_TEXT_SHOWING}, // OP_Tj,
      {" TJ ", 4, OPCAT_TEXT_SHOWING}, // OP_TJ,
      {" ' ",  3, OPCAT_TEXT_SHOWING}, // OP_quote,
      {" \" ", 3, OPCAT_TEXT_SHOWING}, // OP_double_quote
      // OPCAT_TYPE_3_FONTS
      {" d0 ", 4, OPCAT_TYPE_3_FONTS}, // OP_d0,
      {" d1 ", 4, OPCAT_TYPE_3_FONTS}, // OP_d1,
      // OPCAT_COLOR
      {" CS ", 4, OPCAT_COLOR}, // OP_CS,
      {" cs ", 4, OPCAT_COLOR}, // OP_cs,
      {" SC ", 4, OPCAT_COLOR}, // OP_SC,
      {" SCN ",5, OPCAT_COLOR}, // OP_SCN,
      {" sc ", 4, OPCAT_COLOR}, // OP_sc,
      {" scn ",5, OPCAT_COLOR}, // OP_scn,
      {" G ",  3, OPCAT_COLOR}, // OP_G,
      {" g ",  3, OPCAT_COLOR}, // OP_g,
      {" RG ", 4, OPCAT_COLOR}, // OP_RG,
      {" rg ", 4, OPCAT_COLOR}, // OP_rg,
      {" K ",  3, OPCAT_COLOR}, // OP_K,
      {" k ",  3, OPCAT_COLOR}, // OP_k
      // OPCAT_SHADING_PATTERNS
      {" sh ", 4, OPCAT_SHADING_PATTERNS}, // OP_sh,
      // OPCAT_INLINE_IMAGES
      {"BI ",  3, OPCAT_SHADING_PATTERNS}, // OP_BI, -
      {"ID ",  3, OPCAT_SHADING_PATTERNS}, // OP_ID, -
      {"EI ",  3, OPCAT_SHADING_PATTERNS}, // OP_EI, -
      // OPCAT_MARKED_CONTENT
      {" MP ", 4, OPCAT_MARKED_CONTENT}, // OP_MP ,
      {" DP ", 4, OPCAT_MARKED_CONTENT}, // OP_DP ,
      {" BMC ",5, OPCAT_MARKED_CONTENT}, // OP_BMC,
      {" BDC ",5, OPCAT_MARKED_CONTENT}, // OP_BDC,
      {"EMC ", 4, OPCAT_MARKED_CONTENT}, // OP_EMC -
      // OPCAT_COMPATIBILITY
      {"BX ",  3, OPCAT_COMPATIBILITY}, // OP_BX, -
      {"EX ",  3, OPCAT_COMPATIBILITY}, // OP_EX -
      // OPCAT_XOBJECTS
      {" Do ", 4, OPCAT_XOBJECTS | OPCAT_XOBJECT_IMAGE}, // OP_Do_image
      {" Do ", 4, OPCAT_XOBJECTS | OPCAT_XOBJECT_MASK}, // OP_Do_mask
  };


  // ---------------------------------------------------------------------------
  //                Graphics objects state machine
  //
  //
  // Each state is represented by a pointer to its transition function. The
  // transition function accepts a graphics operator and returns the next
  // state. A state is identified by an index into the table of transition
  // functions.
  //

  //
  // Initial graphics object state
  //
  ObjFmtBasic::ContentStateId state_page_description(GraphicsOperator op)
  {
      switch(op)
      {
      case OP_m:
      case OP_re:
          return ObjFmtBasic::STATE_PATH_OBJECT;

      case OP_BT:
          return ObjFmtBasic::STATE_TEXT_OBJECT;

      case OP_BI:
          JAG_TBD;
          break;

      case OP_sh:
      case OP_Do_mask:
      case OP_Do_image:
          // PDF Reference: immediate return to this state
          return ObjFmtBasic::STATE_PAGE_DESCRIPTION;

      default:
          if (!(s_operators[op].category &
                (OPCAT_GENERAL_GRAPHICS_STATE |
                 OPCAT_SPECIAL_GRAPHICS_STATE |
                 OPCAT_COLOR |
                 OPCAT_TEXT_STATE |
                 OPCAT_MARKED_CONTENT)))
          {
              throw exception_invalid_operation() << JAGLOC;
          }
      }
      return ObjFmtBasic::STATE_PAGE_DESCRIPTION;
  }



  //
  //
  //
  ObjFmtBasic::ContentStateId state_text_object(GraphicsOperator op)
  {
      if (op == OP_ET)
          return ObjFmtBasic::STATE_PAGE_DESCRIPTION;

      if (!(s_operators[op].category &
            (OPCAT_GENERAL_GRAPHICS_STATE |
             OPCAT_COLOR |
             OPCAT_TEXT_STATE |
             OPCAT_TEXT_SHOWING |
             OPCAT_TEXT_POSITIONING |
             OPCAT_MARKED_CONTENT)))
      {
          throw exception_invalid_operation() << JAGLOC;
      }
      return ObjFmtBasic::STATE_TEXT_OBJECT;
  }

  //
  //
  //
  ObjFmtBasic::ContentStateId state_clipping_path(GraphicsOperator op)
  {
      if (s_operators[op].category & OPCAT_PATH_PAINTING)
          return ObjFmtBasic::STATE_PAGE_DESCRIPTION;

      throw exception_invalid_operation() << JAGLOC;
  }


  //
  //
  //
  ObjFmtBasic::ContentStateId state_path_object(GraphicsOperator op)
  {
      if (s_operators[op].category & OPCAT_PATH_PAINTING)
          return ObjFmtBasic::STATE_PAGE_DESCRIPTION;

      if ((op == OP_W) || (op == OP_W_star))
          return ObjFmtBasic::STATE_CLIPPING_PATH;

      if (!(s_operators[op].category & OPCAT_PATH_CONSTRUCTION))
          throw exception_invalid_operation() << JAGLOC;

      return ObjFmtBasic::STATE_PATH_OBJECT;
  }

  //
  // table with transition functions
  //
  typedef ObjFmtBasic::ContentStateId (*state_transition_fn)(GraphicsOperator);
  const state_transition_fn s_states[ObjFmtBasic::NUM_STATES] =
  {
      state_page_description,
      state_text_object,
      state_clipping_path,
      state_path_object
  };



//---------------------------------------------------------------------------



///
/// substitution table for non-printing ASCII characters
///
unsigned char const printable_map[256]={
 //00 //01 //02 //03 //04 //05 //06 //07 //08 //09
    1,   1,   1,   1,   1,   1,   1,   1, 'b', 't', //000
  'n',   1, 'f', 'r',   1,   1,   1,   1,   1,   1, //010
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1, //020
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1, //030
  '(', ')',   1,   1,   1,   1,   1,   1,   1,   1, //040
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1, //050
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1, //060
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1, //070
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1, //080
    1,   1,'\\',   1,   1,   1,   1,   1,   1,   1, //090
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1, //100
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1, //110
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1, //120
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1, //130
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1, //140
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1, //150
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1, //160
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1, //170
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1, //180
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1, //190
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1, //200
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1, //210
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1, //220
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1, //230
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1, //240
    1,   1,   1,   1,   1,   1                      //250
};

/**
 * @brief ISeqStreamOutput based string formatting
 *
 * formatting is performed as described in '3.2.3 Literal Strings'
 */
class StringStream
    : public jag::ISeqStreamOutput
{
public:
    /// Ctor; takes the following stream
    StringStream(ISeqStreamOutput& next) : m_next(next) {}

public: //ISeqStreamOutput
    void write(void const* data, jag::ULong size);
    jag::ULong tell() const { return m_next.tell(); }
    void flush() { m_next.flush(); }

private:
    ISeqStreamOutput&    m_next;
};

/// implements string formatting
void StringStream::write(void const* data, jag::ULong size)
{
    unsigned char const *curr = static_cast<unsigned char const*>(data);
    unsigned char const *end = curr + size;
    unsigned char const *first_unprocessed = curr;

    unsigned char buff[2] = { '\\', '?' };
    for(;curr!=end; ++curr)
    {
        if (printable_map[*curr] != 1)
        {
            if (curr != first_unprocessed)
                m_next.write(first_unprocessed, static_cast<jag::UInt>(curr-first_unprocessed));
            buff[1] = printable_map[*curr];
            m_next.write(buff, 2);
            first_unprocessed = curr+1;
        }
    }

    if (curr != first_unprocessed)
        m_next.write(first_unprocessed, static_cast<jag::UInt>(curr-first_unprocessed));
}


bool is_ascii7(jag::Char const* txt, size_t length)
{
    jag::Char const*const end = txt+length;
    while(txt<end && *txt++ >= 0);
    return txt == end;
}

  void hexlify_data(ISeqStreamOutput& out_stream, Char const* txt, size_t length)
  {
      const int buffer_length = 128;
      Char buffer[buffer_length+1];

      for(size_t j=0; j<length;)
      {
          Char *buffer_next = buffer;
          while((j<length) && (buffer+buffer_length!=buffer_next))
          {
              jstd::snprintf(buffer_next, 3, "%02x",
                             static_cast<unsigned int>(
                                 static_cast<unsigned char>(txt[j++])));
              buffer_next+=2;
          }
          out_stream.write(buffer, static_cast<UInt>(buffer_next-buffer));
      }
  }

} //namespace anonymous


/**
 * @brief writes a literal string to a specified stream
 *
 * @param out_stream stream to write
 * @param txt string to write
 * @param length length of the string
 */
void ObjFmtBasic::text_string_internal(ISeqStreamOutput& out_stream, Char const* txt, size_t length)
{
    m_stream->write("(", 1);
    out_stream.write(txt, length);
    m_stream->write(")", 1);
}

/**
* @brief writes a hexadecimal string to a specified stream
*
* @param out_stream stream to write
* @param txt string to write
* @param length length of the string
*/
void ObjFmtBasic::text_string_hex_internal(ISeqStreamOutput& out_stream, Char const* txt, size_t length)
{
    if (!length)
        return;

    m_stream->write("<", 1);
    hexlify_data(out_stream, txt, length);
    m_stream->write(">", 1);
}


/**
* @brief Constructor
*
* @param stream stream where to write
*/
ObjFmtBasic::ObjFmtBasic(ISeqStreamOutput& stream, UnicodeConverterStream& utf8_to_16be)
    : m_stream(&stream)
    , m_enc_stream(m_stream)
    , m_enc_stream_ctrl(0)
    , m_utf8_to_16be(utf8_to_16be)
    , m_state(STATE_PAGE_DESCRIPTION)
    , m_operator_categories(0)
{
    TRACE_DETAIL << "Output formatter created (" << this << ")." ;
}

/**
 * @brief sets an encryption stream
 *
 * @param enc_stream pointer to an encryption stream (NULL means no encryption)
 */
void ObjFmtBasic::encryption_stream(EncryptionStream* enc_stream)
{
    m_enc_stream_ctrl = enc_stream;
    m_enc_stream = enc_stream ? enc_stream : m_stream;
}


/// opens an array
ObjFmtBasic& ObjFmtBasic::array_start()
{
#ifdef JAG_DEBUG
    m_consistency_stack.push(ARRAY);
#endif

    m_stream->write("[", 1);
    return *this;
}

/**
* @brief closes the previously opened array
*
* debug build checks if there really is an array to be closed
*/
ObjFmtBasic& ObjFmtBasic::array_end()
{
#ifdef JAG_DEBUG
    JAG_INVARIANT(m_consistency_stack.top() == ARRAY);
    m_consistency_stack.pop();
#endif

    m_stream->write("]", 1);
    return *this;
}


/// opens a dictionary
ObjFmtBasic& ObjFmtBasic::dict_start()
{
#ifdef JAG_DEBUG
    m_consistency_stack.push(DICT);
#endif

    m_stream->write("<<", 2);
    return *this;
}

/**
* @brief closes the previously opened dictionary
*
* debug build checks if there really is a dictionary to be closed
*/
ObjFmtBasic& ObjFmtBasic::dict_end()
{
#ifdef JAG_DEBUG
    JAG_INVARIANT(m_consistency_stack.top() == DICT);
    m_consistency_stack.pop();
#endif

    m_stream->write(">>", 2);
    return *this;
}

/**
*  @brief Outputs a signed integer value
*
*  @param value value to output
*/
ObjFmtBasic& ObjFmtBasic::output(Int value)
{
    const int buffer_length = std::numeric_limits<Int>::digits10 + 3;
    Char buffer[buffer_length];
    int written = jstd::snprintf(buffer, buffer_length, "%d", value);
    m_stream->write(buffer, written);
    return *this;
}

/**
 * @brief Outputs an unsigned integer value
 *
 * The value as outputted as an unsigned integer < 2^31-1
 *
 * @param value value to output
 */
ObjFmtBasic& ObjFmtBasic::output(UInt value)
{
    // the value is subject PDF speficiation limit:
    // -2^31 <= allowed-integral-value <= 2^31-1
    value = (min)(static_cast<size_t>(0xffffffffu), value);

    const int buffer_length = std::numeric_limits<Int>::digits10 + 3;
    Char buffer[buffer_length];
    int written = jstd::snprintf(buffer, buffer_length, "%d", value);
    m_stream->write(buffer, written);
    return *this;
}





/**
*  @brief Outputs double value
*
*  @param value value to output
*
*  @todo correct output (according to IEEE 754-1985) + unit test
*/
ObjFmtBasic& ObjFmtBasic::output(double value)
{
    if (value == std::numeric_limits<double>::infinity())
    {
        JAG_ASSERT(!"DIAG: +infinity");
    }
    else if (value == -std::numeric_limits<double>::infinity())
    {
        JAG_ASSERT(!"DIAG: -infinity");
    }
    else if (value != value)
    {
        JAG_ASSERT(!"DIAG: qNAN");
    }
    else
    {
        Char buffer[PDF_DOUBLE_MAX_SIZE];
        int written = snprintf_pdf_double(buffer, PDF_DOUBLE_MAX_SIZE, value);
        m_stream->write(buffer, written);
            
//         value = (min)(value,3.403e+38);
//         value = (max)(value,-3.403e+38);
//         const int buffer_length = std::numeric_limits<double>::digits10+10;
//         Char buffer[buffer_length];
//         // A seemingly simple task of printing a double with at most 5
//         // significant decimal digits of precision in fractional part does not
//         // have a straightforward solution:
//         //
//         // std::ostrstream .. deprecated
//         // std::ostringstream .. allocation + slow
//         // boost::format .. slow and maybe allocation
//         // printf %e .. exponent
//         // printf %g .. cannot specify the number of digits after the decimal
//         //              point (%. specifies significant digits); moreover in
//         //              certain cases can print exponent
//         // printf %f .. always prints trailing zeros
//         //
//         //
//         // To get a result consistent across multiple platforms we do the
//         // following:
//         //  - round the number to .00001
//         //  - format it with %f forcing to always print the fractional part (by
//         //    using #)
//         //  - discard the trailing zeroes and possibly the decimal point
//         //
//         double rounded = round(value, .00001);
//         if (rounded == 0.0) rounded = 0.0; // negative zero -> zero
//         int written = jstd::snprintf(buffer,
//                                      buffer_length,
//                                      "%#.5f",
//                                      rounded);
//         char* p = buffer + written - 1;
//         while(*p == '0') --p;
//         if (*p != '.') ++p;
//         m_stream->write(buffer, p-buffer);
    }

    return *this;
}

/**
 *  @brief Outputs string with leading '/'
 *
 *  @param value zero-terminated string to output
 *
 *  no checking of the string contents is performed
 *  should be used for strings known in compile time
 */
ObjFmtBasic& ObjFmtBasic::output(Char const* value)
{
    JAG_PRECONDITION(value && value[0]);
    m_stream->write("/", 1);
    jstd::WriteStringToSeqStream(*m_stream, value);
    return *this;
}

/**
 *  @brief Outputs string with leading '/'
 *
 *  @param value zero-terminated string to output
 *
 *  the string is scanned and some characters are
 *  replaced by #xx, where xx is a hexadecimal number
 *  (see 3.2.4 Name Objects)
 */
ObjFmtBasic& ObjFmtBasic::name(Char const* value)
{
    JAG_PRECONDITION(value && value[0]);
    m_stream->write("/", 1);
    Char const* it = value;
    Char const* last_not_written = it;
    for(;*it; ++it)
    {
        if (*it<33 || *it>126 || *it=='#' || *it=='/')
        {
            const ptrdiff_t nr_not_written = it-last_not_written;
            if (nr_not_written)
                m_stream->write(last_not_written, nr_not_written);

            Char buff[4];
            sprintf(buff, "#%02x", static_cast<unsigned char>(*it));
            m_stream->write(buff, 3);
            last_not_written = it+1;
        }
    }
    const ptrdiff_t nr_not_written = it-last_not_written;
    if (nr_not_written)
        m_stream->write(last_not_written, nr_not_written);

    return *this;
}


ObjFmtBasic& ObjFmtBasic::rectangle(
    Double llx, Double lly, Double urx, Double ury)
{
    array_start();
    output(llx);space();
    output(lly);space();
    output(urx);space();
    output(ury);
    array_end();
    return *this;
}

ObjFmtBasic& ObjFmtBasic::null()
{
    raw_bytes("null", 4);
    return *this;
}

//////////////////////////////////////////////////////////////////////////
ObjFmtBasic& ObjFmtBasic::output_bool(bool value)
{
    return raw_text(value ? "true " : "false ");
}


//
//
//
ObjFmtBasic& ObjFmtBasic::output(jstd::trans_affine_t const& value)
{
    Double const* data = value.data();
    array_start();
    output(data[0]).space();
    output(data[1]).space();
    output(data[2]).space();
    output(data[3]).space();
    output(data[4]).space();
    output(data[5]);
    return *this;
}


//////////////////////////////////////////////////////////////////////////
ObjFmtBasic& ObjFmtBasic::output_hex(UInt value, size_t bytes)
{
    const size_t max_bytes = 4;
    JAG_PRECONDITION(bytes && bytes<=max_bytes);
    const size_t buffer_length = 1+2+max_bytes*2; // <xx>
    Char buffer[buffer_length];
    Char* curr = buffer;

    *curr++ = '<';
    for(;bytes--; curr+=2)
        jstd::snprintf(curr, 3, "%02x", (value>>(bytes*8)) &0xff);

    *curr++ = '>';
    m_stream->write(buffer, curr-buffer);
    return *this;
}




//explicit specialization
template ObjFmtBasic& ObjFmtBasic::output_resource(ColorSpaceHandle const& handle);
template ObjFmtBasic& ObjFmtBasic::output_resource(ImageHandle const& handle);
template ObjFmtBasic& ObjFmtBasic::output_resource(GraphicsStateHandle const& handle);
template ObjFmtBasic& ObjFmtBasic::output_resource(PatternHandle const& handle);
template ObjFmtBasic& ObjFmtBasic::output_resource(ShadingHandle const& handle);




/**
*  @brief Outputs dictionary key
*
*  The same functionality as output(Char const* value);
*  it exists mainly for explicit expression of the fact that
*  a dictionary key is being outputted.
*
*  @param key zero-terminated key to output
*/
ObjFmtBasic& ObjFmtBasic::dict_key(Char const* key)
{
    return output(key);
}

/**
* @brief writes string as it is passed (without any modification)
*
* if you know the string's length, use raw_bytes() instead
*
* param value zero terminated string to write
*/
ObjFmtBasic& ObjFmtBasic::raw_text(Char const* value)
{
    jstd::WriteStringToSeqStream(*m_stream, value);
    return *this;
}

/// writes a space
ObjFmtBasic& ObjFmtBasic::space()
{
    m_stream->write(" ", 1);
    return *this;
}

/**
* @brief writes raw data
*
* @param data data to write
* @param length length of the data
*/
ObjFmtBasic& ObjFmtBasic::raw_bytes(void const* data, size_t length)
{
    m_stream->write(data, length);
    return *this;
}

/**
 * @brief writes a stream data
 *
 * If there is an active encryption stream, then the passed data
 * are encrypted using this stream.
 */
ObjFmtBasic& ObjFmtBasic::stream_data(void const* data, size_t length)
{
    m_enc_stream->write(data, length);
    return *this;
}



/**
* @brief writes a string
*
* If there is an active encryption stream then the string is encrypted.
*
* @param txt string to output
* @param length length of the string
*/
ObjFmtBasic& ObjFmtBasic::text_string(Char const* txt, size_t length, bool is_utf8)
{
    bool recode = is_utf8
        ? !is_ascii7(txt, length)
        : false;

    if (!m_enc_stream_ctrl)
    {
        StringStream str_stream(*m_stream);
        ISeqStreamOutput* out_stream = &str_stream;
        if (recode)
        {
            m_utf8_to_16be.reset(&str_stream, true);
            out_stream = &m_utf8_to_16be;
        }
        text_string_internal(*out_stream, txt, length);
    }
    else
    {
        m_enc_stream_ctrl->reseed();
        StringStream str_stream(m_enc_stream_ctrl->top_stream());
        m_enc_stream_ctrl->push_stream(str_stream);
        ON_BLOCK_EXIT_OBJ(*m_enc_stream_ctrl, &EncryptionStream::pop_stream);

        ISeqStreamOutput* out_stream = m_enc_stream;
        if (recode)
        {
            m_utf8_to_16be.reset(m_enc_stream, true);
            out_stream = &m_utf8_to_16be;
        }
        text_string_internal(*out_stream, txt, length);
    }
    return *this;
}

/**
* @brief writes a string
*
* If there is an active encryption stream then the string is encrypted.
*
* @param txt zero-terminated string to output
*/
ObjFmtBasic& ObjFmtBasic::text_string(Char const* txt)
{
    return text_string(txt, static_cast<int>(strlen(txt)));
}

ObjFmtBasic& ObjFmtBasic::text_string(std::string const& txt, bool is_utf8)
{
    return text_string(txt.c_str(), txt.size(), is_utf8);
}


/**
* @brief writes a hexadecimal string
*
* If there is an active encryption stream then the string is encrypted.
*
* @param txt string to output
* @param length length of the string
*/
ObjFmtBasic& ObjFmtBasic::text_string_hex(Char const* txt, size_t length)
{
    if (!length)
        return *this;

    if (!m_enc_stream_ctrl) {
        text_string_hex_internal(*m_stream, txt, length);
    }
    else {
        //
        // encrypt the string to a temporary buffer and hexlify the buffer to
        // the output strem as a byte string
        //

        // encrypt the string
        MemoryStreamOutput mem_stream;
        m_enc_stream_ctrl->push_stream(mem_stream);
        ON_BLOCK_EXIT_OBJ(*m_enc_stream_ctrl, &EncryptionStream::pop_stream);
        m_enc_stream_ctrl->write(txt, length);
        JAG_ASSERT(length == mem_stream.tell());


        // hexlify encrypted data
        Char const* encrypted =
            jag_reinterpret_cast<Char const*>(mem_stream.shared_data().get());
        m_stream->write("<", 1);
        hexlify_data(*m_stream, encrypted, length);
        m_stream->write(">", 1);
    }
    return *this;
}

/**
* @brief writes a unencrypted string
*
* Even if there is an active encryption stream then the string is unencrypted.
*
* @param txt string to output
* @param length length of the string
*/
ObjFmtBasic& ObjFmtBasic::unenc_text_string(Char const* txt, size_t length)
{
    StringStream str_stream(*m_stream);
    text_string_internal(str_stream, txt, length);
    return *this;
}

/**
* @brief writes a unencrypted string
*
* Even if there is an active encryption stream then the string is unencrypted.
*
* @param txt zero-terminated string to output
*/
ObjFmtBasic& ObjFmtBasic::unenc_text_string(Char const* txt)
{
    unenc_text_string(txt, static_cast<int>(strlen(txt)));
    return *this;
}

/**
* @brief writes a unencrypted hexadecimal string
*
* Even if there is an active encryption stream then the string is unencrypted.
*
* @param txt string to output
* @param length length of the string
*/
ObjFmtBasic& ObjFmtBasic::unenc_text_string_hex(Char const* txt, size_t length)
{
    text_string_hex_internal(*m_stream, txt, length);
    return *this;
}

/**
 * @brief writes a sequence of gids
 * @param sequence of gids
 * @param length string *character* length
 *
 * string within a content stream
 */
ObjFmtBasic& ObjFmtBasic::string_object_2b(UInt16 const* txt, size_t length)
{
    JAG_PRECONDITION(length);
    const int buffer_size = 32;
    char buffer[buffer_size];

    m_stream->write("<", 1);
    for(UInt16 const*const end=txt+length; txt!=end; ++txt)
    {
        jstd::snprintf(buffer, buffer_size, "%04x", (unsigned)*txt);
        m_stream->write(buffer, 4);
    }
    m_stream->write(">", 1);
    return *this;
}

/**
 * @brief writes a sequence of bytes as a string
 * @param start sequence start
 * @param sequence end
 */
ObjFmtBasic& ObjFmtBasic::string_object_2b(Char const* start, Char const* end)
{
    JAG_TBD; // not actually tested

    JAG_PRECONDITION(start != end);
    JAG_PRECONDITION(!((end-start)%2));

    const int buffer_size = 64;
    char buffer[buffer_size];
    char* curr=buffer;


    m_stream->write("<", 1);
    for(; start!=end; ++start, curr+=2)
    {
        if (curr==end)
        {
            m_stream->write(buffer, buffer_size);
            curr=buffer;
        }
        jstd::snprintf(curr, buffer_size, "%02x", static_cast<int>(*start));
    }
    if (curr!=buffer)
        m_stream->write(buffer, curr-buffer);
    m_stream->write(">", 1);
    return *this;
}



/// destructor, in debug builds it checks whether there is no array or dictionary still opened
ObjFmtBasic::~ObjFmtBasic()
{
#ifdef JAG_DEBUG
    TRACE_ERR.if_(!m_consistency_stack.empty()) << "ObjFmt: array/dict not closed." ;
    while (!m_consistency_stack.empty())
    {
        //OUTPUT_TYPE tmp = m_consistency_stack.top();
        m_consistency_stack.pop();
    }
#endif
    TRACE_DETAIL << "Output formatter released (" << this << ")." ;
}


//
//
//
ObjFmtBasic& ObjFmtBasic::graphics_op(GraphicsOperator op)
{
    m_state = s_states[m_state](op);
    OperatorRecord const& op_rec = s_operators[op];
    m_operator_categories |= op_rec.category;
    m_stream->write(op_rec.name, op_rec.length);
    JAG_ASSERT(strlen(op_rec.name) == static_cast<std::size_t>(op_rec.length));
    return *this;
}

//
// Copies the formatter state (not the content of the underlying streams).
//
// This is intended for taking a *read-only* snapshot of the content stream.
// 
void ObjFmtBasic::copy_to(ObjFmtBasic& fmt) const
{
    fmt.m_state = m_state;
    fmt.m_operator_categories = m_operator_categories;
}


//////////////////////////////////////////////////////////////////////////
// DateWriter
DateWriter::DateWriter()
{
    time_t ltime;
    time(&ltime);
    struct tm *today = localtime(&ltime);
    strftime(m_date, DateWriter::BUFFER_SIZE, "D:%Y%m%d%H%M%S", today);
}




}} //namespace jag::pdf
