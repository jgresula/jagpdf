// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//

#ifndef __MACROS_CPP__1221110
#define __MACROS_CPP__1221110

namespace jag_detail {
template<class T> inline void ignore_unused_variable_warning(const T&) {}
}

/// use for deliberately unused arguments
//#define JAG_UNUSED_FUNCTION_ARGUMENT(arg) arg;
//#define JAG_UNUSED_VARIABLE(arg) arg;

#define JAG_UNUSED_FUNCTION_ARGUMENT(arg) ::jag_detail::ignore_unused_variable_warning(arg);
#define JAG_UNUSED_VARIABLE(arg) ::jag_detail::ignore_unused_variable_warning(arg);

#endif //__MACROS_CPP__1221110
