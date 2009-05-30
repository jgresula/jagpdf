// Copyright (c) 2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)

#include "precompiled.h"
#include <core/jstd/encodinghelpers.h>
#include <core/generic/assert.h>
#include <string.h>

namespace jag {
namespace jstd {

namespace
{
  char const* g_enc_map[ENC_NUM_ITEMS];

  // initializes g_enc_map
  struct InitEncodingMap
  {
      InitEncodingMap()
      {
          memset(g_enc_map, 0, sizeof(g_enc_map));
          g_enc_map[ENC_CP_1250] = "ibm-5346_P100-1998";
          g_enc_map[ENC_CP_1251] = "ibm-5347_P100-1998";
          g_enc_map[ENC_CP_1252] = "ibm-5348_P100-1997";
          g_enc_map[ENC_CP_1253] = "ibm-5349_P100-1998";
          g_enc_map[ENC_CP_1254] = "ibm-5350_P100-1998";
          g_enc_map[ENC_CP_1255] = "ibm-9447_P100-2002";
          g_enc_map[ENC_CP_1256] = "ibm-9448_X100-2005";
          g_enc_map[ENC_CP_1257] = "ibm-9449_P100-2002";
          g_enc_map[ENC_CP_1258] = "ibm-5354_P100-1998";
          g_enc_map[ENC_MAC_ROMAN] = "macos-0_2-10.2";
          g_enc_map[ENC_PDF_STANDARD] = "ibm-1276_P100-1995";
          g_enc_map[ENC_ISO_8859_1] = "ISO-8859-1";
          g_enc_map[ENC_ISO_8859_2] = "ibm-912_P100-1995";
          g_enc_map[ENC_ISO_8859_3] = "ibm-913_P100-2000";
          g_enc_map[ENC_ISO_8859_4] = "ibm-914_P100-1995";
          g_enc_map[ENC_ISO_8859_5] = "ibm-915_P100-1995";
          g_enc_map[ENC_ISO_8859_6] = "ibm-1089_P100-1995";
          g_enc_map[ENC_ISO_8859_7] = "ibm-9005_X110-2007";
          g_enc_map[ENC_ISO_8859_8] = "ibm-5012_P100-1999";
          g_enc_map[ENC_ISO_8859_9] = "ibm-920_P100-1995";
          g_enc_map[ENC_ISO_8859_10] = "iso-8859_10-1998";
          g_enc_map[ENC_ISO_8859_11] = "iso-8859_11-2001";
          g_enc_map[ENC_ISO_8859_13] = "ibm-921_P100-1995";
          g_enc_map[ENC_ISO_8859_14] = "iso-8859_14-1998";
          g_enc_map[ENC_ISO_8859_15] = "ibm-923_P100-1998";
          g_enc_map[ENC_UTF_8] = "UTF-8";
          g_enc_map[ENC_SHIFT_JIS] = "shift_jis";
          g_enc_map[ENC_GB2312] = "gb2312";
          g_enc_map[ENC_HANGEUL] = "ks_c_5601-1987";
          g_enc_map[ENC_BIG5] = "big5";
      }
  } g_enc_map_init;

} // anonymous namespace


//
//
//
char const* encoding_from_id(EnumCharacterEncoding enc)
{
    JAG_ASSERT(g_enc_map[enc]);
    return g_enc_map[enc];
}

}} // namespace jag::jstd

/** EOF @file */
