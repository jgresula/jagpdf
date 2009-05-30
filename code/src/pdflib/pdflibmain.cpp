// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//



#include "precompiled.h"
#include <core/jstd/tssdllmain.h>


// The library implementation is in an archive. On linux, unless you specify
// --whole-archive linker option the archive is discarded as the DSO does not
// refer to any symbol in the archive. Currently, it is not possible to use this
// option due to limitations in boost.build
//
// The alternative is to have this file that explicitly refers to symbols in the
// archive and is compiled as part of DSO (i.e. not part of the archive). It
// seems, it is enough to refer to just one symbol.
#include "capiruntime.h"

void *jag_pdf_archive_importer = reinterpret_cast<void*>(&jag_create_profile_from_file);
void *importer = reinterpret_cast<void*>(&jag::jstd::tls_cleanup_implemented);



/** EOF @file */
