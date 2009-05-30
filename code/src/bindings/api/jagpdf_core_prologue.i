// -*-c++-*-
//
// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


// disable unknown base warning
%warnfilter(401) jag::apiinternal::StreamOut;
%warnfilter(401) jag::IProfile;
%warnfilter(401) jag::IPage;
%warnfilter(401) jag::IDocument;
%warnfilter(401) jag::ICanvas;
%warnfilter(401) jag::IDocumentOutline;
%warnfilter(401) jag::IFont;
%warnfilter(401) jag::IImageDef;
%warnfilter(401) jag::IImage;
%warnfilter(401) jag::IImageMask;

%include jag_exception.swg
%include smartptr.swg

%nodefaultctor;


