// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <pdflib/cfgsymbols.h>
#include <interfaces/execcontext.h>

// this file contains definition of pdf configuration options
namespace
{
  jag::jstd::cfg_pair_t const config_defaults[] =
  {
      // general
      {"doc.version"           , "5"},
      {"doc.encryption"        , ""},
      {"doc.static_file_id"    , "0"},  // not-documented
      {"doc.compressed"        , "1"},
      {"doc.strict_mode"       , "1"},
      {"doc.trace_level"       , "1"},  // not-documented
      {"doc.trace_show_loc"    , "0"},  // not-documented

      {"doc.page_layout"       , ""},
      {"doc.page_mode"         , ""},
      {"doc.initial_destination"      , ""},
      {"doc.viewer_preferences", ""},
      {"doc.topdown",            "0"},

      // should not be used directly but via get_default_text_encoding()
      {"text.encoding"         , ""},   // not-documented
      {"text.kerning"          , "0"},

      // patterns
      {"patterns.tiling_type"      , "1"},

      // fonts
      {"fonts.embedded"            , "1"},
      {"fonts.fallback"            , ""}, // not-documented
      {"fonts.synthesized"         , "1"},
      {"fonts.subset"              , "1"},
      {"fonts.force_cid"           , "1"},
      {"fonts.default"             , "standard;name=Helvetica;size=12"},

      // images
      {"images.interpolated"       , "0"},
      {"images.intent"             , "-1"}, // not-documented
      {"images.default_dpi"        , "72"},
      {"images.softmask_16_to_8"   , "1"},  // not-documented
      {"images.png_advanced_color" , "1"},  // not-documented

      // info
      {"info.title"                , ""},
      {"info.author"               , ""},
      {"info.subject"              , ""},
      {"info.keywords"             , ""},
      {"info.creator"              , ""},
      {"info.creation_date"        , "1"},  // not-documented
      {"info.static_producer"      , "0"},  // not-documented

      // standard security handler
      {"stdsh.pwd_owner"           , ""},
      {"stdsh.pwd_user"            , ""},
      {"stdsh.permissions"         , ""},
      {0                           , 0}
  };

} // anonymous namespace

namespace jag {
namespace pdf {

extern jstd::cfg_pair_t const* s_config_symbols = config_defaults;


// Holds the text default encoding. It overrides 'text.encoding' value. It
// is usually set by clients (e.g. Java wrapper) that cannot supply text in
// various encodings.
extern char const* s_default_text_encoding = 0;

//
// Must be used when asking for "text.encoding".
//
char const* get_default_text_encoding(IExecContext const& exec_ctx)
{
    if (s_default_text_encoding)
        return s_default_text_encoding;

    return exec_ctx.config().get("text.encoding");
}


}} // namespace jag::pdf

/** EOF @file */
