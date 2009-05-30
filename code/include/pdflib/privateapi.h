// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef PRIVATEAPI_JG2031_H__
#define PRIVATEAPI_JG2031_H__

#include <jagpdf/detail/c_prologue.h>
#include <iosfwd>

namespace jag {

class exception;
JAG_EXPORT void JAG_CALLSPEC format_exception_message(exception const& exc_, std::ostream& stream);

} // namespace jag

#endif // PRIVATEAPI_JG2031_H__
/** EOF @file */
