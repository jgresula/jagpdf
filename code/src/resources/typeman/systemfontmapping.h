// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __SYSTEMFONTMAPPING_H_JAG_1310__
#define __SYSTEMFONTMAPPING_H_JAG_1310__

#include <boost/shared_ptr.hpp>
#include <boost/intrusive_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H


namespace jag {
namespace resources {
class FontSpecImpl;
class TypefaceImpl;
struct CharEncodingRecord;

///
/**
 * @brief Finds a typeface for given font spec by invoking system font mapping.
 *
 * @param ftlib freetype lib
 * @param specimpl font specification
 *
 * @pre specimpl::facename must not be empty
 *
 * @return typeface or null if no typeface is not found
 */
std::auto_ptr<TypefaceImpl> typeface_from_system(
    boost::shared_ptr<FT_LibraryRec_> ftlib
    , FontSpecImpl const* specimpl
    , CharEncodingRecord const& enc_rec);

}} // namespace jag::resources

#endif //__SYSTEMFONTMAPPING_H_JAG_1310__

