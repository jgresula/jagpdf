// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef MMAP_JG1428_H__
#define MMAP_JG1428_H__

const int JAG_MMAP_VIEW_DEFAULT_SIZE = 512*1024;

#ifdef _WIN32
# include "win32/mmap_win32.h"
#else
# include "other/mmap_other.h"
#endif

#endif // MMAP_JG1428_H__
/** EOF @file */
