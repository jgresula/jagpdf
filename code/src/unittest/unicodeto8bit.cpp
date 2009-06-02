// Copyright (c) 2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)

#include "testtools.h"
#include <core/generic/scopeguard.h>
#include <core/jstd/unicode.h>
#include <core/jstd/encodinghelpers.h>
#include <unicode/ucnv.h>
#include <unicode/ustring.h>
#include <assert.h>
#include <string.h>

using namespace jag::jstd;
using namespace jag;

namespace {

// this string contains characters from windows-1250 and ISO-8859-5
const char* utf_8_str = "\xc4\x8c\xc3\xad\xc4\x8d""a!\xd0\x96\xd1\x86\xd0\xba";

void icu_tests()
{
    UErrorCode err = U_ZERO_ERROR;
    UConverter* cnv_utf8 = ucnv_open("utf-8", &err);
    ON_BLOCK_EXIT(ucnv_close, cnv_utf8);
    UConverter* cnv_1250 = ucnv_open("windows-1250", &err);
    ON_BLOCK_EXIT(ucnv_close, cnv_1250);
    ucnv_setFromUCallBack(cnv_1250, UCNV_FROM_U_CALLBACK_STOP, 0, 0, 0, &err);

    const char* src = utf_8_str;
    const char* src_end = utf_8_str + strlen(utf_8_str);
    const int buff_len = 64;
    char out_buff[buff_len];
    memset(out_buff, 0, buff_len);

    while(src < src_end)
    {
        char const* prev_src = src;

        // get next codepoint
        UChar32 target = ucnv_getNextUChar(cnv_utf8, &src, src_end, &err);
        assert(!err);

        // convert it to UChars
        UChar uchar_buff[2];
        int32_t uchar_buff_len;
        u_strFromUTF32(uchar_buff, 2, &uchar_buff_len, &target, 1, &err);
        assert(!err);


        // convert it to codepage
        ucnv_fromUChars(cnv_1250,
                        out_buff, buff_len,
                        uchar_buff, uchar_buff_len,
                        &err);

        if (err == U_INVALID_CHAR_FOUND)
        {
            src = prev_src;
            break;
        }

        if (U_FAILURE(err))
            assert(0);
    }

    // test that src points to the first non-1250 character
    BOOST_TEST(src == utf_8_str + 8);

    printf("src_end-src_start: %d\n", utf_8_str + strlen(utf_8_str) - src);
    printf("orig: %s\n", utf_8_str);
    printf("left: %s\n", src);
}

//
//
//
void verify_consistent_strings(UnicodeToCPIterator& it,
                              const size_t lengths[],
                              const EnumCharacterEncoding encodings[],
                              int length)
{
    std::vector<jag::Char> str8;

    for(int i=0;;++i)
    {
        EnumCharacterEncoding* enc = it.next(str8);
        if (!enc) {
            BOOST_TEST(i == length);
            break;
        }

        BOOST_TEST(lengths[i] == str8.size());
        BOOST_TEST(encodings[i] == *enc);

        std::string s_8bit(&str8[0], &str8[0] + str8.size());
        printf("%s | %s\n", s_8bit.c_str(), encoding_from_id(*enc));
    }
}


//
//
//
void basic_test()
{
    EnumCharacterEncoding encs[3] =
        {ENC_CP_1252, ENC_CP_1250, ENC_CP_1251};

    UnicodeToCP cnv(encs, encs+3);

    // empty string test
    UnicodeToCPIterator it_emptystr(
        cnv.create_iterator(utf_8_str, utf_8_str));
    verify_consistent_strings(it_emptystr, 0, 0, 0);


    // simple test
    UnicodeToCPIterator it(
        cnv.create_iterator(utf_8_str, utf_8_str + strlen(utf_8_str)));

    size_t lengths[2] = {5, 3};
    EnumCharacterEncoding encodings[2] = {ENC_CP_1250, ENC_CP_1251};

    verify_consistent_strings(it, lengths, encodings, 2);


    // encoding changes
    const char* str_chg = "\xc4\x8c\xd0\x96\xc4\x8c\xd0\x96\xc4\x8c\xd0\x96";
    UnicodeToCPIterator it_chg(
        cnv.create_iterator(str_chg, str_chg + strlen(str_chg)));

    size_t len_chg[6] = {1, 1, 1, 1, 1, 1};
    EnumCharacterEncoding enc_chg[6] = {ENC_CP_1250, ENC_CP_1251,
                                        ENC_CP_1250, ENC_CP_1251,
                                        ENC_CP_1250, ENC_CP_1251};
    verify_consistent_strings(it_chg, len_chg, enc_chg, 6);

}


//
//
//
void test()
{
    icu_tests();
    basic_test();
}


} // anonymous namespace





//
//
//
int unicodeto8bit(int, char** const)
{
    int result = guarded_test_run(test);
    result += boost::report_errors();
    return result;
}


/** EOF @file */
