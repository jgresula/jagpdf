// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "precompiled.h"
#include <pdflib/apifreefuns.h>
#include "docwriterimpl.h"
#include <core/jstd/file_stream.h>
#include <core/jstd/configimpl.h>
#include <core/jstd/externalstreamwrap.h>
#include <core/generic/refcountedimpl.h>
#include <core/errlib/except.h>
#include <core/jstd/mmap.h>
#include <core/generic/checked_cast.h>
#include <interfaces/configuration.h>
#include <pdflib/cfgsymbols.h>
#include <jagpdf/detail/version.h>

#include <boost/mem_fn.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/intrusive_ptr.hpp>

#include <iostream>
#include <msg_errlib.h>


using namespace boost;

namespace jag {

//////////////////////////////////////////////////////////////////////////
intrusive_ptr<IDocument>
JAG_CALLSPEC create_file(Char const* file_path, intrusive_ptr<IProfile> config)
{
    // create a file stream
    shared_ptr<ISeqStreamOutputControl> file_stream (
        new jstd::MMapFileStreamOutput(file_path)
   );

    // prepare a config object
    intrusive_ptr<IProfileInternal> cfg_internal = config
        ? checked_static_pointer_cast<IProfileInternal>(config)
        : intrusive_ptr<IProfileInternal>(new RefCountImpl<jstd::ConfigImpl,RefCountMT>(pdf::s_config_symbols));

    return new RefCountImpl<pdf::DocWriterImpl>(file_stream, cfg_internal);
}

//
//
//
intrusive_ptr<IDocument>
JAG_CALLSPEC create_stream(jag_streamout const* stream, intrusive_ptr<IProfile> config)
{
    // create a wrapper around the external stream
    shared_ptr<ISeqStreamOutputControl> wrapped_stream (
        new jstd::ExternalStreamOut(stream)
       );

    // prepare a config object
    intrusive_ptr<IProfileInternal> cfg_internal = config
        ? checked_static_pointer_cast<IProfileInternal>(config)
        : intrusive_ptr<IProfileInternal>(new RefCountImpl<jstd::ConfigImpl,RefCountMT>(pdf::s_config_symbols));

    return new RefCountImpl<pdf::DocWriterImpl>(wrapped_stream, cfg_internal);
}

//
//
//
intrusive_ptr<IDocument>
JAG_CALLSPEC create_stream(apiinternal::StreamOut* stream, intrusive_ptr<IProfile> config)
{
//     puts("attach");
//     getc(stdin);

    // create a wrapper around the external stream
    shared_ptr<ISeqStreamOutputControl> wrapped_stream (
        new jstd::ExternalSWIGStreamOut(stream)
       );

    // prepare a config object
    intrusive_ptr<IProfileInternal> cfg_internal = config
        ? checked_static_pointer_cast<IProfileInternal>(config)
        : intrusive_ptr<IProfileInternal>(new RefCountImpl<jstd::ConfigImpl,RefCountMT>(pdf::s_config_symbols));

    return new RefCountImpl<pdf::DocWriterImpl>(wrapped_stream, cfg_internal);
}



//////////////////////////////////////////////////////////////////////////
intrusive_ptr<IProfile> JAG_CALLSPEC create_profile()
{
     return new RefCountImpl<jstd::ConfigImpl,RefCountMT>(pdf::s_config_symbols);
}


//
//
//
boost::intrusive_ptr<IProfile>
JAG_CALLSPEC create_profile_from_string(Char const* str)
{
    return load_from_string(str, pdf::s_config_symbols);
}


//
//
//
boost::intrusive_ptr<IProfile>
JAG_CALLSPEC create_profile_from_file(Char const* fname)
{
    return load_from_file(fname, pdf::s_config_symbols);
}

///
/// @brief Returns library version.
///
/// The version is stored in the lowest 24 bits.
///
/// - b0 -b7   patch level
/// - b8 -b15  minor version
/// - b16-b23  major version
UInt
JAG_CALLSPEC version()
{
    return pdf::this_version;
}



///////////////////////////////////////////////////////////////////////////
// private functions
void JAG_CALLSPEC format_exception_message(exception const& exc_, std::ostream& stream)
{
    output_exception(exc_, stream);
}

namespace detail
{
  void throw_virtual_not_implemented(char const* fun_desc)
  {
      throw exception_operation_failed(
          msg_virtual_fun_not_implemented_s(fun_desc)) << JAGLOC;
  }
}

} //namespace jag



