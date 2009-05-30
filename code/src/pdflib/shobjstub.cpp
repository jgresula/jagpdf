// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#include "capiruntime.h"


namespace jag { namespace jstd {
void tls_cleanup_implemented();
}}

void *jag_pdf_archive_importer = reinterpret_cast<void*>(&jag_create_profile_from_file);
void *importer = reinterpret_cast<void*>(&jag::jstd::tls_cleanup_implemented);


/** EOF @file */
