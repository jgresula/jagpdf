// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __CHARENCODING_H_JAG_2003__
#define __CHARENCODING_H_JAG_2003__


namespace jag {

enum EnumCharacterEncoding
{
    // WinNT single-byte encodings
    EN_FIRST_RECOGNIZED_BY_LOGFONT
    , ENC_CP_1250 = EN_FIRST_RECOGNIZED_BY_LOGFONT
                        // WinLatin2, CE        windows-1250    EASTEUROPE_CHARSET
    , ENC_CP_1251       // WinCyrillic          windows-1251    RUSSIAN_CHARSET
    , ENC_CP_1252       // WinAnsi, WinLatin1   windows-1252    ANSI_CHARSET
    , ENC_CP_1253       // Greek                windows-1253    GREEK_CHARSET
    , ENC_CP_1254       // Turkish              windows-1254    TURKISH_CHARSET
    , ENC_CP_1255       // Hebrew               windows-1255    HEBREW_CHARSET
    , ENC_CP_1256       // Arabic               windows-1256    ARABIC_CHARSET
    , ENC_CP_1257       // Baltic               windows-1257    BALTIC_CHARSET
    , ENC_CP_1258       // Vietnamese           windows-1258    VIETNAMESE_CHARSET
    , ENC_MAC_ROMAN     //                      macintosh       MAC_CHARSET

    // multi-byte encodings supported on WinNT
    , ENC_SHIFT_JIS     //win ext:Windows-31J / shift_jis       SHIFTJIS_CHARSET    cp 932
    , ENC_GB2312        //                      gb2312          GB2312_CHARSET         cp 936
    , ENC_HANGEUL       //                      ks_c_5601-1987  HANGEUL_CHARSET     cp 949
    , ENC_BIG5          //                      big5            CHINESEBIG5_CHARSET cp 950

    , EN_LAST_RECOGNIZED_BY_LOGFONT = ENC_BIG5


    //////////////////////////////////////////////////////////////////////////

    , ENC_PDF_STANDARD  //                      Adobe-Standard-Encoding

    // ISO 8859-x single-byte encodings
    , ENC_ISO_8859_1    // Latin-1              ISO_8859-1:1987
    , ENC_ISO_8859_2    // Latin-2              ISO_8859-2:1987
    , ENC_ISO_8859_3    // Latin-3              ISO_8859-3:1988
    , ENC_ISO_8859_4    // Latin-4              ISO_8859-4:1988
    , ENC_ISO_8859_5    // Latin/Cyrillic       ISO_8859-5:1988
    , ENC_ISO_8859_6    // Latin/Arabic         ISO_8859-6:1987
    , ENC_ISO_8859_7    // Latin/Greek          ISO_8859-7:1987
    , ENC_ISO_8859_8    // Latin/Hebrew         ISO_8859-8:1988
    , ENC_ISO_8859_9    // Latin-5              ISO_8859-9:1989
    , ENC_ISO_8859_10   // Latin-6              ISO-8859-10
    , ENC_ISO_8859_11   // Latin/Thai           ISO-8859-11
    , ENC_ISO_8859_13   // Latin-7              ISO-8859-13
    , ENC_ISO_8859_14   // Latin-8              ISO-8859-14
    , ENC_ISO_8859_15   // Latin-9              ISO-8859-15
    , ENC_ISO_8859_16   // Latin-10             ISO-8859-16

    , ENC_IDENTITY

    // unicode encodings
    , ENC_UTF_8         // UTF-8
    , ENC_UTF_7         // UTF-7
    , ENC_UTF_16        // UTF-16
    , ENC_UTF_16BE      // UTF-16BE
    , ENC_UTF_16LE      // UTF-16LE

    // built-in encoding
    , ENC_BUILTIN


    , ENC_NUM_ITEMS
};

} //namespace jag


#endif //__CHARENCODING_H_JAG_2003__

