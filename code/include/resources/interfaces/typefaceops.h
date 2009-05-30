// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef TYPEFACEOPS_JG1944_H__
#define TYPEFACEOPS_JG1944_H__

namespace jag {

class ITypeface;

size_t hash_value(ITypeface const& font);
bool operator<(ITypeface const& lhs, ITypeface const& rhs);
bool operator==(ITypeface const& lhs, ITypeface const& rhs);
inline bool operator!=(ITypeface const& lhs, ITypeface const& rhs) { return !(lhs==rhs); }

} // namespace jag

#endif // TYPEFACEOPS_JG1944_H__
/** EOF @file */
