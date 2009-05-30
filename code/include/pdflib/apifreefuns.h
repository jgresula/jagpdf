// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef APIFREEFUNS_JG2353_H__
#define APIFREEFUNS_JG2353_H__

#include <jagpdf/detail/c_prologue.h>
#include <pdflib/apistructures.h>
#include <boost/intrusive_ptr.hpp>
#include <interfaces/configuration.h>

namespace jag {

class IProfile;
class IDocument;

/// Creates a default profile.
///
/// The retrieved profile instance is [ref_langspecific_t reference counted].
///
/// @see section [ref_profile].
boost::intrusive_ptr<IProfile> JAG_CALLSPEC create_profile();

/// Creates a profile from a string.
///
/// The retrieved profile instance is [ref_langspecific_t reference counted].
///
/// @see section [ref_profile].
boost::intrusive_ptr<IProfile> JAG_CALLSPEC create_profile_from_string(Char const* str);

/// Creates a profile from a file
///
/// The retrieved profile instance is [ref_langspecific_t reference counted].
///
/// @see section [ref_profile].
boost::intrusive_ptr<IProfile> JAG_CALLSPEC create_profile_from_file(Char const* fname);


/// Retrieves [lib] version.
///
/// The [lib] version triplet is encoded into a single value as follows:
///  - bits 0-7  - patch
///  - bits 8-15 - minor
///  - bits 16-23 - major
///
/// Not to be confused with [code_document_version].
///
/// @see section [ref_versioning]
UInt JAG_CALLSPEC version();


/// Creates a PDF document which will be written to a file.
///
/// The retrieved document instance is [ref_langspecific_t reference counted].
///
/// @param file_path destination file
/// @param profile profile to be used
///
boost::intrusive_ptr<IDocument> JAG_CALLSPEC create_file(Char const* file_path, boost::intrusive_ptr<IProfile> profile=boost::intrusive_ptr<IProfile>());

#if !defined(SWIG)
  // do not let SWIG see this declaration, it is intended for GCCXML,
  // i.e. for C/C++ api

  /// Creates a PDF document which will be written to a stream.
  ///
  /// The retrieved profile instance is [ref_langspecific_t reference counted].
  ///
  /// @param stream destination stream
  /// @param profile profile to be used
  ///
  /// @see section [ref_customstreams].
  boost::intrusive_ptr<IDocument> JAG_CALLSPEC create_stream(jag_streamout const* stream, boost::intrusive_ptr<IProfile> profile=boost::intrusive_ptr<IProfile>());
#endif

#if !defined(__GCCXML__) && !defined(__DOXYGEN__)
  // do not let GCCXML and doxygen see this as this only for SWIG purposes
  boost::intrusive_ptr<IDocument> JAG_CALLSPEC create_stream(jag::apiinternal::StreamOut* stream, boost::intrusive_ptr<IProfile> profile=boost::intrusive_ptr<IProfile>());
#endif

} // namespace jag

#endif // APIFREEFUNS_JG2353_H__
/** EOF @file */
