// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//

#include"../systemfontmapping.h"
#include <resources/typeman/typemanimpl.h>
#include <core/errlib/errlib.h>
#include <msg_resources.h>

namespace jag {
namespace resources {

class TypefaceImpl;
class FontSpecImpl;
struct CharEncodingRecord;

//
//
//
std::auto_ptr<TypefaceImpl> typeface_from_system(
    boost::shared_ptr<FT_LibraryRec_> /*ftlib*/
    , FontSpecImpl const* /*specimpl*/
    , CharEncodingRecord const& /*enc_rec*/)
{
    // not supported
    return std::auto_ptr<TypefaceImpl>();
}


}} // namespace jag::resources

/** EOF @file */
