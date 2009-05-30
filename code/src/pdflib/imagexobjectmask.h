// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __IMAGEXOBJECTMASK_H_JG_1810__
#define __IMAGEXOBJECTMASK_H_JG_1810__

#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include "indirectobjectfwd.h"
#include <boost/scoped_ptr.hpp>

namespace jag
{

class IImageMaskData;

namespace pdf {

class DocWriterImpl;
class ContentStream;
class ObjFmt;

class ImageXObjectMask
    : public IndirectObjectFwd
{
public:
    DEFINE_VISITABLE;
    ImageXObjectMask(DocWriterImpl& doc, IImageMaskData const& img_mask_spec);

private:
    void on_write(ObjFmt& fmt);
    void on_before_output();

private:
    IImageMaskData const&             m_img_mask;
    boost::scoped_ptr<ContentStream>  m_content_stream;
    DocWriterImpl&                    m_doc;
    unsigned                          m_actual_bpc;
    unsigned                          m_mask_type;
};

}} //namespace jag::pdf


#endif //__IMAGEXOBJECTMASK_H_JG_1810__


