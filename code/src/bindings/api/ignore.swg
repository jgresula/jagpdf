// -*-c++-*-

// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


// NOTE: This file is parsed not only by SWIG, but also by a custom
// script that fetches ignore definitions. So the line can be one of
// the following:
//  - an empty line
//  - line starting with '//'
//  - %ignore definition

//
// ignored stuff
//
%ignore jag::ICanvas::text_r;
%ignore jag::ICanvas::text_ro;
%ignore jag::ICanvas::text_simple_r;
%ignore jag::ICanvas::text_simple_ro;
%ignore jag::ICanvas::text_simple_ro;
%ignore jag::IFont::advance_r;
