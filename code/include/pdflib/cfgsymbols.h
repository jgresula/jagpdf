// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef CFGSYMBOLS_JG2044_H__
#define CFGSYMBOLS_JG2044_H__

#include <core/jstd/configimpl.h>

namespace jag {

class IExecContext;

namespace pdf {

extern jstd::cfg_pair_t const* s_config_symbols;
extern char const* s_default_text_encoding;

char const* get_default_text_encoding(IExecContext const& exec_ctx);

}} // namespace jag::pdf

#endif // CFGSYMBOLS_JG2044_H__
/** EOF @file */
