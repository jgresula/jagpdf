// -*-c++-*-

// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


%include rename.swg
%include ignore.swg
%include generated.swg

%define INCLUDE_FILE(header)
%{
#include header
%}
%enddef

%define EXPORT_FILE(header)
INCLUDE_FILE(header)
%include header
%enddef

%header
%{
#include <interfaces/stdtypes.h>
#include <interfaces/constants.h>
#include <resources/interfaces/charencoding.h>
#include <pdflib/apistructures.h>

#if defined(_MSC_VER)
// ignore some bogus warnings
# pragma warning(disable:4244)
# pragma warning(disable:4505)
# pragma warning(disable:4706)
#else
# include <dlfcn.h>
#endif
%}

%include <interfaces/stdtypes.h>
%include <interfaces/constants.h>
%include <pdflib/apistructures.h>


%include files.swg

