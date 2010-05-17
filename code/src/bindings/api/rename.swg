// -*-c++-*-
//
// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


// NOTE: This file is parsed not only by SWIG, but also by a custom script that
// fetches rename definitions and applies them to C++ API. So the line can be
// one of the following:
//  - an empty line
//  - line starting with '//'
//  - %rename definition

// IPDFCanvas
%rename(text) jag::ICanvas::text_r;
%rename(text) jag::ICanvas::text_o;
%rename(text) jag::ICanvas::text_ro;
%rename(text) jag::ICanvas::text_simple;
%rename(text) jag::ICanvas::text_simple_o;
%rename(text) jag::ICanvas::text_simple_r;
%rename(text) jag::ICanvas::text_simple_ro;

%rename(text_glyphs) jag::ICanvas::text_glyphs_o;

%rename(color) jag::ICanvas::color1;
%rename(color) jag::ICanvas::color3;
%rename(color) jag::ICanvas::color4;
%rename(pattern) jag::ICanvas::pattern1;
%rename(pattern) jag::ICanvas::pattern3;
%rename(pattern) jag::ICanvas::pattern4;

%rename(shading_pattern_load) jag::IDocument::shading_pattern_load_n;

// IDocumentOutline
%rename(item) jag::IDocumentOutline::item_destination;
%rename(item) jag::IDocumentOutline::item_destination_obj;

// IPage
%rename(annotation_goto) jag::IPage::annotation_goto_obj;

%rename(advance) jag::IFont::advance_r;
