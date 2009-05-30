// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <core/jstd/unicode.h>
#include <core/jstd/tracer.h>
#include <core/generic/assert.h>
#include <core/errlib/errlib.h>
#include <core/jstd/icumain.h>
#include <core/jstd/encodinghelpers.h>
#include <unicode/ustring.h>
#include <boost/static_assert.hpp>

namespace jag {
namespace jstd {

//
//
//
char const* get_canonical_converter_name(Char const* enc)
{
    static char const * const standards[] = {
        "IANA",
        "UTR22", // internal names
        "",
        0
    };


    UErrorCode err = U_ZERO_ERROR;
    for(char const*const* s=&standards[0]; *s; ++s)
    {
        if (char const* canonical=ucnv_getCanonicalName(enc, *s, &err))
            return canonical;

        CHECK_ICU(err);
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////
UnicodeConverter::UnicodeConverter(Char const* encoding)
{
    UErrorCode err = U_ZERO_ERROR;
    TRACE_DETAIL << "Encoding converter " << encoding << " created (" << this << ")." ;
    m_converter = ucnv_open(encoding, &err);
    CHECK_ICU(err);
    JAG_POSTCONDITION(m_converter);
}


//////////////////////////////////////////////////////////////////////////
UnicodeConverter::~UnicodeConverter()
{
    TRACE_DETAIL << "Encoding converter released (" << this << ")." ;
    ucnv_close(m_converter);
}


//////////////////////////////////////////////////////////////////////////
UnicodeConverter::UnicodeConverter(UConverter const* other)
{
    UErrorCode err = U_ZERO_ERROR;
    m_converter = ucnv_safeClone(other, 0, 0, &err);
    CHECK_ICU(err);
    JAG_POSTCONDITION(m_converter);
}


//////////////////////////////////////////////////////////////////////////
std::auto_ptr<UnicodeConverter> UnicodeConverter::clone() const
{
    return std::auto_ptr<UnicodeConverter>(new UnicodeConverter(m_converter));
}

//////////////////////////////////////////////////////////////////////////
void UnicodeConverter::reset()
{
    ucnv_reset(m_converter);
}


//////////////////////////////////////////////////////////////////////////
Int UnicodeConverter::next_code_point(Char const** source, Char const* end)
{
    UErrorCode err = U_ZERO_ERROR;
    Int result = ucnv_getNextUChar(m_converter, source, end, &err);
    CHECK_ICU(err);
    return result;
}



Int UnicodeConverter::from_codepoint(Int codepoint, Char* buffer, unsigned buffer_size)
{
    UErrorCode err = U_ZERO_ERROR;

    UChar uchar_buffer[2];
    int32_t dest_len;
    u_strFromUTF32(uchar_buffer, 2, &dest_len, &codepoint, 1, &err); CHECK_ICU(err);
    int32_t result = ucnv_fromUChars(m_converter, buffer, buffer_size, uchar_buffer, dest_len, &err); CHECK_ICU(err);
    return result;
}



///////////////////////////////////////////////////////////////////////////
// class UnicodeConverterStream


static const Byte BOM_UTF16BE[2] = { 0xfe, 0xff };

UnicodeConverterStream::UnicodeConverterStream(
    Char const* src,
    Char const* dst,
    ISeqStreamOutput* out,
    bool write_bom
)
    : m_src_conv(src)
    , m_dst_conv(dst)
    , m_out_stream(out)
    , m_pos(0)
    , m_bom(write_bom)
{
    // currently, there is only use case requiring utf-16be BOM so no
    // overengineering here
    JAG_PRECONDITION(!m_bom ||
                      (UCNV_UTF16_BigEndian == ucnv_getType(m_dst_conv.conv_internal())));
}


/**
 * @brief Resets converter state.
 *
 * @param out   output stream
 */
void UnicodeConverterStream::reset(ISeqStreamOutput* out, bool write_bom)
{
    m_out_stream = out;
    m_pos = 0;
    m_bom = write_bom;
}


/**
 * @brief Writes data (in src encoding) to the following stream (in target encoding).
 *
 * @param data  data to write (source encoding); the data must be 'complete' - e.g.
 *              in case of UTF-16 even number of bytes must be supplied
 * @param size  data lenght
 */
void UnicodeConverterStream::write(void const* data, ULong size)
{
    if (!size)
        return;

    const int CHUNK_SIZE = 256;
    Char target_buffer[CHUNK_SIZE];
    UChar pivot_buffer[CHUNK_SIZE];

    Char const*const target_end = target_buffer+CHUNK_SIZE;

    Char const* src_buffer = static_cast<Char const*>(data);
    Char const*const src_end = src_buffer + size;

    UChar *pivot_src, *pivot_target;
    pivot_src = pivot_target = pivot_buffer;
    UChar const*const pivot_end = pivot_buffer + CHUNK_SIZE;

    JAG_ASSERT_MSG(m_out_stream, "output stream not set");

    //handle BOM - hardwired for BOM_UTF16BE (see comment in ctor)
    if (m_pos==0 && m_bom)
        m_out_stream->write(BOM_UTF16BE, 2);

    do
    {
        UErrorCode err = U_ZERO_ERROR;
        Char* target = target_buffer;

        ucnv_convertEx(m_dst_conv.conv_internal(),
                        m_src_conv.conv_internal(),
                        &target,
                        target_end,
                        &src_buffer,
                        src_end,
                        pivot_buffer,
                        &pivot_src,
                        &pivot_target,
                        pivot_end,
                        FALSE,
                        TRUE,
                        &err);

        if ((err!=U_BUFFER_OVERFLOW_ERROR) && U_FAILURE(err))
        {
            m_src_conv.reset();
            m_dst_conv.reset();
            CHECK_ICU(err);
            JAG_INTERNAL_ERROR;
        }

        ptrdiff_t length = target-target_buffer;
        m_out_stream->write(target_buffer, length);
        m_pos += length;
    }
    while(src_buffer != src_end);
}




// ---------------------------------------------------------------------------
//                    class UnicodeToCPIterator
//

//
//
//
UnicodeToCPIterator::UnicodeToCPIterator(UnicodeToCP& obj,
                                         Char const* begin,
                                         Char const* end)
    : m_obj(&obj)
    , m_current(begin)
    , m_end(end)
{}


//
// Places the next consistent string to 'str' and returns its codepage. Returns
// NULL when done.
//
EnumCharacterEncoding *UnicodeToCPIterator::next(std::vector<Char>& str)
{
    if (m_current >= m_end)
        return 0;

    str.resize(0);

    for(;;)
    {
        UnicodeToCP::List& records = m_obj->m_records;
        UnicodeToCP::RecordIter it_end = records.end();
        UnicodeToCP::RecordIter it = records.begin();
        for(; it != it_end; ++it)
        {
            consistent_string(it->conv, str);
            if (!str.empty())
            {
                if (it != records.begin())
                    records.splice(
                        records.begin(), records, it);

                return &it->enc;
            }
        }

        // Retrieve converter for the next unused encoding. Instruct the
        // converter to stop on invalid char.
        for(;;)
        {
            if (m_obj->m_begin_enc >= m_obj->m_end_enc)
            {
                // The character was not found in any specified codepage; remove
                // the next codepoint from the input, issue a warning and use
                // something else instead
                UErrorCode err = U_ZERO_ERROR;
                UChar32 target = ucnv_getNextUChar(m_obj->m_cnv_utf8,
                                                   &m_current, m_end,
                                                   &err);
                str.push_back('.');
                TRACE_WRN << "No codepage found for code point " << target << '.';
                break;
            }

            UErrorCode err = U_ZERO_ERROR;
            EnumCharacterEncoding enc = *m_obj->m_begin_enc++;
            UConverter* cnv = ucnv_open(encoding_from_id(enc), &err);
            if (U_SUCCESS(err))
            {
                ucnv_setFromUCallBack(cnv, UCNV_FROM_U_CALLBACK_STOP, 0, 0, 0, &err);
                records.push_front(UnicodeToCP::rec_t(enc, cnv));
                break;
            }
        }
    }
}

//
//
//
void UnicodeToCPIterator::consistent_string(UConverter* cnv,
                                            std::vector<Char>& str)
{
    UErrorCode err = U_ZERO_ERROR;
    while(m_current < m_end)
    {
        char const* prev_src = m_current;

        // get next codepoint
        UChar32 target = ucnv_getNextUChar(m_obj->m_cnv_utf8,
                                           &m_current, m_end,
                                           &err);
        CHECK_ICU(err);

        // convert it to UChars
        UChar uchar_buff[2];
        int32_t uchar_buff_len;
        u_strFromUTF32(uchar_buff, 2, &uchar_buff_len,
                       &target, 1,
                       &err);
        CHECK_ICU(err);

        // convert it to codepage
        char out;
        ucnv_fromUChars(cnv,
                        &out, 1,
                        uchar_buff, uchar_buff_len,
                        &err);

        if (err == U_INVALID_CHAR_FOUND)
        {
            m_current = prev_src;
            break;
        }

        CHECK_ICU_FATAL(err);
        str.push_back(out);
    }
}


// ---------------------------------------------------------------------------
//                    class UnicodeToCP


//
//
//
UnicodeToCP::UnicodeToCP(EnumCharacterEncoding const* begin,
                             EnumCharacterEncoding const* end)
    : m_begin_enc(begin)
    , m_end_enc(end)
{
    UErrorCode err = U_ZERO_ERROR;
    m_cnv_utf8 = ucnv_open("utf-8", &err);

    CHECK_ICU_FATAL(err);
}

//
//
//
UnicodeToCP::~UnicodeToCP()
{
    ucnv_close(m_cnv_utf8);

    RecordIter end = m_records.end();
    for(RecordIter it = m_records.begin(); it != end; ++it)
    {
        JAG_ASSERT(it->conv);
        ucnv_close(it->conv);
    }
}

//
//
//
UnicodeToCPIterator UnicodeToCP::create_iterator(Char const* begin,
                                                     Char const* end)
{
    return UnicodeToCPIterator(*this, begin, end);
}



}} //namespace jag::jstd
